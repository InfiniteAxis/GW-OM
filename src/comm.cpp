/***********************************************************************
*                    Gundam Wing: Operation Meteor                     *
*----------------------------------------------------------------------*
* The recent additions and changes to this code (SWR 1.0), not         *
* including the original code itself is here by copyrighted.           *
* Under US federal and government regulations, any use of the          *
* additions or changes not part of the original code are subject       *
* to such fines as seeing fit by the government of the United          *
* United States of America. [Code Changes from SWR 1.0 to GW:OM        *
*----------------------------------------------------------------------*
* Additions to SWR (1.0) for Gundam Operation Meteor are               *
* copyrighted to - Justin Bedder (Cray/Trowa) 2001+ (c)                *
*----------------------------------------------------------------------*
* SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                       *
* SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                *
* Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops            *
* ---------------------------------------------------------------------*
* Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael    *
* Chastain, Michael Quan, and Mitchell Tse.                            *
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,      *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe. *
***********************************************************************/


#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "mud.h"
#include "sha256.h"
#include "mccp.h"

/*
 * Socket and TCP/IP stuff.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>

#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];


const unsigned char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const unsigned char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const unsigned char go_ahead_str[] = { IAC, GA, '\0' };

void cleanup_memory( void );
void save_sysdata( SYSTEM_DATA sys );
void save_banlist( void );
#define STRING_NONE               0

/*  from act_info?  */
void show_condition( CHAR_DATA *ch, CHAR_DATA *victim );

/*
 * Global variables.
 */
IMMORTAL_HOST *immortal_host_start; /* Start of Immortal legal domains */
IMMORTAL_HOST *immortal_host_end;   /* End of Immortal legal domains */
DESCRIPTOR_DATA *first_descriptor;  /* First descriptor     */
DESCRIPTOR_DATA *last_descriptor;   /* Last descriptor      */
DESCRIPTOR_DATA *d_next;    /* Next descriptor in loop  */
int num_descriptors;
bool mud_down;  /* Shutdown         */
bool wizlock;   /* Game is wizlocked        */
time_t boot_time;
HOUR_MIN_SEC set_boot_time_struct;
HOUR_MIN_SEC *set_boot_time;
struct tm *new_boot_time;
struct tm new_boot_struct;
char str_boot_time[MAX_INPUT_LENGTH];
char lastplayercmd[MAX_INPUT_LENGTH * 2];
time_t current_time;    /* Time of this pulse       */
int port;   /* Port number */
int control;    /* Controlling descriptor   */
int newdesc;    /* New descriptor       */
fd_set in_set;  /* Set of desc's for reading    */
fd_set out_set; /* Set of desc's for writing    */
fd_set exc_set; /* Set of desc's with errors    */
int maxdesc;

/*
 * OS-dependent local functions.
 */
void game_loop( );
int init_socket( int gport );
void new_descriptor( int new_desc );
bool read_from_descriptor( DESCRIPTOR_DATA *d );

/*
 * Other local functions (OS-independent).
 */
bool check_parse_name( const char *name );
short check_reconnect( DESCRIPTOR_DATA *d, const char *name, bool fConn );
short check_playing( DESCRIPTOR_DATA *d, const char *name, bool kick );
bool check_multi( DESCRIPTOR_DATA *d, const char *name );
int main( int argc, char **argv );
void nanny( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_name( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_old_password( DESCRIPTOR_DATA *d, const char *argument );
void nanny_confirm_new_name( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_new_password( DESCRIPTOR_DATA *d, const char *argument );
void nanny_confirm_new_password( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_new_sex( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_new_race( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_new_class( DESCRIPTOR_DATA *d, const char *argument );
void nanny_roll_stats( DESCRIPTOR_DATA *d, const char *argument );
void nanny_stats_ok( DESCRIPTOR_DATA *d, const char *argument );
void nanny_press_enter( DESCRIPTOR_DATA *d, const char *argument );
void nanny_read_motd( DESCRIPTOR_DATA *d, const char *argument );
void nanny_confirm_agreement( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_hair( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_highlights( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_eye_color( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_build( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_hero( DESCRIPTOR_DATA *d, const char *argument );
void nanny_get_invis( DESCRIPTOR_DATA *d, const char *argument );
bool flush_buffer( DESCRIPTOR_DATA *d, bool fPrompt );
void read_from_buffer( DESCRIPTOR_DATA *d );
void stop_idling( CHAR_DATA *ch );
void free_desc( DESCRIPTOR_DATA *d );
void display_prompt( DESCRIPTOR_DATA *d );
void set_pager_input( DESCRIPTOR_DATA *d, const char *argument );
bool pager_output( DESCRIPTOR_DATA *d );

void mail_count( CHAR_DATA *ch );

int main( int argc, char **argv )
{
	struct timeval now_time;
	/*    int port; */
	bool fCopyOver = false;

	/*
	 * Memory debugging if needed.
	 */
#if defined(MALLOC_DEBUG)
	malloc_debug( 2 );
#endif

	num_descriptors = 0;
	first_descriptor = NULL;
	last_descriptor = NULL;
	sysdata.NO_NAME_RESOLVING = true;
	sysdata.WAIT_FOR_AUTH = true;
	mudstrlcpy( lastplayercmd, "No commands issued yet", MSL );

	/*
	 * Init time.
	 */
	gettimeofday( &now_time, NULL );
	current_time = ( time_t ) now_time.tv_sec;
	/*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
	boot_time = time( 0 );   /*  <-- I think this is what you wanted */
	mudstrlcpy( str_boot_time, ctime( &current_time ), MSL );

	/*
	 * Init boot time.
	 */
	set_boot_time = &set_boot_time_struct;
	/*
	 * set_boot_time->hour   = 6;
	 * set_boot_time->min    = 0;
	 * set_boot_time->sec    = 0;
	 */
	set_boot_time->manual = 0;

	new_boot_time = update_time( localtime( &current_time ) );
	/*
	 * Copies *new_boot_time to new_boot_struct, and then points
	 * new_boot_time to new_boot_struct again. -- Alty
	 */
	new_boot_struct = *new_boot_time;
	new_boot_time = &new_boot_struct;
	new_boot_time->tm_mday += 1;
	if( new_boot_time->tm_hour > 12 )
		new_boot_time->tm_mday += 1;
	new_boot_time->tm_sec = 0;
	new_boot_time->tm_min = 0;
	new_boot_time->tm_hour = 6;

	/*
	 * Update new_boot_time (due to day increment)
	 */
	new_boot_time = update_time( new_boot_time );
	new_boot_struct = *new_boot_time;
	new_boot_time = &new_boot_struct;

	/*
	 * Set reboot time string for do_time
	 */
	get_reboot_string( );

	/*
	 * Get the port number.
	 */
	port = 1984;
	if( argc > 1 )
	{
		if( !is_number( argv[1] ) )
		{
			fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
			exit( 1 );
		}
		else if( ( port = atoi( argv[1] ) ) <= 1024 )
		{
			fprintf( stderr, "Port number must be above 1024.\n" );
			exit( 1 );
		}
		if( argv[2] && argv[2][0] )
		{
			fCopyOver = TRUE;
			control = atoi( argv[3] );
#ifdef IMC
			imcsocket = atoi( argv[4] );
#endif
		}
		else
			fCopyOver = FALSE;
	}

	/*
	 * Run the game.
	 */
	log_string( "Booting Database" );
	boot_db( fCopyOver );
	log_string( "Initializing socket" );
	if( !fCopyOver )
		control = init_socket( port );

	snprintf( log_buf, MAX_STRING_LENGTH, "GW:OM ready on port %d.", port );
	log_string( log_buf );
	if( fCopyOver )
	{
		log_string( "Initiating hotboot recovery." );
		hotboot_recover( );
	}
	game_loop( );
	close( control );

	/*
	 * That's all, folks.
	 */
	log_string( "Normal termination of game." );
	log_string( "Cleaning up Memory." );
	cleanup_memory( );
	exit( 0 );
	return 0;
}


int init_socket( int gport )
{
	char hostname[64];
	struct sockaddr_in sa;
	int x = 1;
	int fd;

	gethostname( hostname, sizeof( hostname ) );


	if( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		perror( "Init_socket: socket" );
		exit( 16 );
	}

	if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( void * ) &x, sizeof( x ) ) < 0 )
	{
		perror( "Init_socket: SO_REUSEADDR" );
		close( fd );
		exit( 17 );
	}

#if defined(SO_DONTLINGER) && !defined(SYSV)
	{
		struct linger ld;

		ld.l_onoff = 1;
		ld.l_linger = 1000;

		if( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER, ( void * ) &ld, sizeof( ld ) ) < 0 )
		{
			perror( "Init_socket: SO_DONTLINGER" );
			close( fd );
			exit( 18 );
		}
	}
#endif

	memset( &sa, '\0', sizeof( sa ) );
	sa.sin_family = AF_INET; /* hp->h_addrtype; */
	sa.sin_port = htons( gport );

	if( bind( fd, ( struct sockaddr * ) &sa, sizeof( sa ) ) == -1 )
	{
		perror( "Init_socket: bind" );
		close( fd );
		exit( 19 );
	}

	if( listen( fd, 50 ) < 0 )
	{
		perror( "Init_socket: listen" );
		close( fd );
		exit( 20 );
	}

	return fd;
}

/*
static void SegVio()
{
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
	snprintf( buf, MAX_STRING_LENGTH, "%cPC: %-20s room: %d", IS_NPC(ch) ? 'N' : ' ',
			ch->name, ch->in_room->vnum );
	log_string( buf );
  }
  exit(0);
}
*/

int child_exit_status = 0;

static void clean_up_child_process( int signal_number )
{
	/* Clean up the child process. */
	int status;
	wait( &status );

	/* Store its exit status in a global variable. */
	child_exit_status = status;
}

/*
 * LAG alarm!							-Thoric
 */
static void caught_alarm( int signum )
{
	char buf[MAX_STRING_LENGTH];
	bug( "ALARM CLOCK!" );
	mudstrlcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\r\n", MSL );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	if( newdesc )
	{
		FD_CLR( newdesc, &in_set );
		FD_CLR( newdesc, &out_set );
		log_string( "clearing newdesc" );
	}
	game_loop( );
	close( control );

	log_string( "Normal termination of game." );
	exit( 0 );
}

bool check_bad_desc( int desc )
{
	if( FD_ISSET( desc, &exc_set ) )
	{
		FD_CLR( desc, &in_set );
		FD_CLR( desc, &out_set );
		log_string( "Bad FD caught and disposed." );
		return true;
	}
	return false;
}

void accept_new( int ctrl )
{
	static struct timeval null_time;
	DESCRIPTOR_DATA *d;
	/*
	 * int maxdesc; Moved up for use with id.c as extern
	 */

#if defined(MALLOC_DEBUG)
	if( malloc_verify( ) != 1 )
		abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( ctrl, &in_set );
	maxdesc = ctrl;
	newdesc = 0;
	for( d = first_descriptor; d; d = d->next )
	{
		maxdesc = UMAX( maxdesc, d->descriptor );
		FD_SET( d->descriptor, &in_set );
		FD_SET( d->descriptor, &out_set );
		FD_SET( d->descriptor, &exc_set );
		if( d->ifd != -1 && d->ipid != -1 )
		{
			maxdesc = UMAX( maxdesc, d->ifd );
			FD_SET( d->ifd, &in_set );
		}
		if( d == last_descriptor )
			break;
	}

	if( select( maxdesc + 1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
		perror( "accept_new: select: poll" );
		exit( 1 );
	}

	if( FD_ISSET( ctrl, &exc_set ) )
	{
		bug( "%s: Exception raise on controlling descriptor %d", __func__, ctrl );
		FD_CLR( ctrl, &in_set );
		FD_CLR( ctrl, &out_set );
	}
	else if( FD_ISSET( ctrl, &in_set ) )
	{
		newdesc = ctrl;
		new_descriptor( newdesc );
	}
}

void game_loop( )
{
	struct timeval last_time;
	char cmdline[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	/*  time_t	last_check = 0;  */

#ifndef WIN32
	signal( SIGPIPE, SIG_IGN );
	signal( SIGALRM, caught_alarm );
	signal( SIGCHLD, clean_up_child_process );
#endif
	/*
	 * signal( SIGSEGV, SegVio );
	 */
	gettimeofday( &last_time, NULL );
	current_time = ( time_t ) last_time.tv_sec;

	/*
	 * Main loop
	 */
	while( !mud_down )
	{
		accept_new( control );
		 /*
		  * Kick out descriptors with raised exceptions
		  * or have been idle, then check for input.
		  */
		for( d = first_descriptor; d; d = d_next )
		{
			if( d == d->next )
			{
				bug( "descriptor_loop: loop found & fixed" );
				d->next = NULL;
			}
			d_next = d->next;

			d->idle++; /* make it so a descriptor can idle out */
			if( FD_ISSET( d->descriptor, &exc_set ) )
			{
				FD_CLR( d->descriptor, &in_set );
				FD_CLR( d->descriptor, &out_set );
				if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
					save_char_obj( d->character );
				d->outtop = 0;
				close_socket( d, true );
				continue;
			}
			else if( !IS_IMMORTAL( d->character ) && ( ( !d->character && d->idle > 360 )  /* 2 mins */
				|| ( d->connected != CON_PLAYING && d->idle > 1200 )    /* 5 mins */
				|| d->idle > 6000 ) )   /* 25 mins */
			{
				write_to_descriptor( d, "\n\rYou've been idle too long, *waves* byebye!\r\n", 0 );
				d->outtop = 0;
				close_socket( d, true );
				continue;
			}
			else
			{
				d->fcommand = false;

				if( FD_ISSET( d->descriptor, &in_set ) )
				{
					d->idle = 0;
					if( d->character )
						d->character->timer = 0;
					if( !read_from_descriptor( d ) )
					{
						FD_CLR( d->descriptor, &out_set );
						if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
							save_char_obj( d->character );
						d->outtop = 0;
						close_socket( d, false );
						continue;
					}
				}

				/* check for input from the dns */
				if( ( d->connected == CON_PLAYING || d->character != NULL ) && d->ifd != -1 && FD_ISSET( d->ifd, &in_set ) )
					process_dns( d );

				if( d->character && d->character->wait > 0 )
				{
					--d->character->wait;
					continue;
				}

				read_from_buffer( d );
				if( d->incomm[0] != '\0' )
				{
					d->fcommand = true;
					stop_idling( d->character );

					mudstrlcpy( cmdline, d->incomm, MSL );
					d->incomm[0] = '\0';

					if( d->character )
						set_cur_char( d->character );

					if( d->pagepoint )
						set_pager_input( d, cmdline );
					else
						switch( d->connected )
						{
						default:
							nanny( d, cmdline );
							break;
						case CON_PLAYING:
							if( d->original )
								d->original->pcdata->cmd_recurse = 0;
							else
								d->character->pcdata->cmd_recurse = 0;
							interpret( d->character, cmdline );
							break;
						case CON_EDITING:
							edit_buffer( d->character, cmdline );
							break;
						}
				}
			}
			if( d == last_descriptor )
				break;
		}

		/*
		 * Autonomous game motion.
		 */
		update_handler( );

		/*
		 * Output.
		 */
		for( d = first_descriptor; d; d = d_next )
		{
			d_next = d->next;

			if( ( d->fcommand || d->outtop > 0 ) && FD_ISSET( d->descriptor, &out_set ) )
			{
				if( d->pagepoint )
				{
					if( !pager_output( d ) )
					{
						if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
							save_char_obj( d->character );
						d->outtop = 0;
						close_socket( d, false );
					}
				}
				else if( !flush_buffer( d, true ) )
				{
					if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
						save_char_obj( d->character );
					d->outtop = 0;
					close_socket( d, false );
				}
			}
			if( d == last_descriptor )
				break;
		}

		/*
		 * Synchronize to a clock.
		 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		 * Careful here of signed versus unsigned arithmetic.
		 */
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;

			gettimeofday( &now_time, NULL );
			usecDelta = ( ( int ) last_time.tv_usec ) - ( ( int ) now_time.tv_usec ) + 1000000 / PULSE_PER_SECOND;
			secDelta = ( ( int ) last_time.tv_sec ) - ( ( int ) now_time.tv_sec );
			while( usecDelta < 0 )
			{
				usecDelta += 1000000;
				secDelta -= 1;
			}

			while( usecDelta >= 1000000 )
			{
				usecDelta -= 1000000;
				secDelta += 1;
			}

			if( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
			{
				struct timeval stall_time;

				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec = secDelta;
				if( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
				{
					perror( "game_loop: select: stall" );
					//exit( 22 );
				}
			}
		}

		gettimeofday( &last_time, NULL );
		current_time = ( time_t ) last_time.tv_sec;

		/*
		 * Check every 5 seconds...  (don't need it right now)
		 * if ( last_check+5 < current_time )
		 * {
		 * CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
		 * DESCRIPTOR_DATA);
		 * last_check = current_time;
		 * }
		 */
	}
	return;
}

void init_descriptor( DESCRIPTOR_DATA *dnew, int desc )
{
	dnew->next = NULL;
	dnew->descriptor = desc;
	dnew->connected = CON_GET_NAME;
	dnew->ansi = true;
	dnew->outsize = 2000;
	dnew->idle = 0;
	dnew->lines = 0;
	dnew->scrlen = 24;
	dnew->user = STRALLOC( "unknown" );
	dnew->newstate = 0;
	dnew->prevcolor = 0x07;
	dnew->ifd = -1; /* Descriptor pipes, used for DNS resolution and such */
	dnew->ipid = -1;
	dnew->can_compress = false;
	CREATE( dnew->mccp, MCCP, 1 );
	dnew->mccp->out_compress = NULL;

	CREATE( dnew->outbuf, char, dnew->outsize );
}

void show_file_to_desc( DESCRIPTOR_DATA *d, const char *filename )
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	int c;
	int num = 0;

	if( ( fp = FileOpen( filename, "r" ) ) != NULL )
	{
		while( !feof( fp ) )
		{
			while( ( buf[num] = fgetc( fp ) ) != EOF
				&& buf[num] != '\n' && buf[num] != '\r' && num < ( MAX_STRING_LENGTH - 2 ) )
				num++;
			c = fgetc( fp );
			if( ( c != '\n' && c != '\r' ) || c == buf[num] )
				ungetc( c, fp );
			buf[num++] = '\n';
			buf[num++] = '\r';
			buf[num] = '\0';
			send_to_desc_color( buf, d );
			num = 0;
		}
		FileClose( fp );
	}
}

void send_greeting( DESCRIPTOR_DATA *d )
{
	char filename[256];

	snprintf( filename, 256, "%sgreeting.dat", MOTD_DIR );
	show_file_to_desc( d, filename );
	d->ansi = true;
}

void new_descriptor( int new_desc )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *dnew;
	BAN_DATA *pban;
	struct sockaddr_in sock;
	int desc;
	socklen_t size;

	set_alarm( 20 );
	size = sizeof( sock );
	if( check_bad_desc( new_desc ) )
	{
		set_alarm( 0 );
		return;
	}

	set_alarm( 20 );
	if( ( desc = accept( new_desc, ( struct sockaddr * ) &sock, &size ) ) < 0 )
	{
		perror( "New_descriptor: accept" );
		set_alarm( 0 );
		return;
	}

	if( check_bad_desc( new_desc ) )
	{
		set_alarm( 0 );
		return;
	}
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

	set_alarm( 20 );
	if( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
	{
		perror( "New_descriptor: fcntl: FNDELAY" );
		set_alarm( 0 );
		return;
	}
	if( check_bad_desc( new_desc ) )
		return;

	CREATE( dnew, DESCRIPTOR_DATA, 1 );
	init_descriptor( dnew, desc );
	dnew->port = ntohs( sock.sin_port );
	mudstrlcpy( log_buf, inet_ntoa( sock.sin_addr ), MSL );
	log_printf_plus( LOG_COMM, LEVEL_OWNER, "Incoming Connection: %s, port %d.", log_buf, dnew->port );
	dnew->host = STRALLOC( log_buf );
	if( !sysdata.NO_NAME_RESOLVING )
	{
		mudstrlcpy( buf, in_dns_cache( log_buf ), MAX_STRING_LENGTH );

		if( buf[0] == '\0' )
			resolve_dns( dnew, sock.sin_addr.s_addr );
		else
		{
			STRFREE( dnew->host );
			dnew->host = STRALLOC( buf );
		}
	}

	for( pban = first_ban; pban; pban = pban->next )
	{
		if( ( !str_prefix( pban->name, dnew->host ) || !str_suffix( pban->name, dnew->host ) ) && pban->level >= LEVEL_OWNER )
		{
			write_to_descriptor( dnew, "Your site has been banned from this Mud.\r\n", 0 );
			free_desc( dnew );
			set_alarm( 0 );
			return;
		}
	}

	/*
	 * Init descriptor data.
	 */
	if( !last_descriptor && first_descriptor )
	{
		DESCRIPTOR_DATA *d;

		bug( "%s: last_desc is NULL, but first_desc is not! ...fixing", __func__ );
		for( d = first_descriptor; d; d = d->next )
			if( !d->next )
				last_descriptor = d;
	}

	LINK( dnew, first_descriptor, last_descriptor, next, prev );

	/*
	 * MCCP Compression
	 */
	write_to_buffer( dnew, ( const char * ) will_compress2_str, 0 );

	/*
	 * Send the greeting.
	 */
	send_greeting( dnew );

	if( ++num_descriptors > sysdata.maxplayers )
		sysdata.maxplayers = num_descriptors;
	if( sysdata.maxplayers > sysdata.alltimemax )
	{
		if( sysdata.time_of_max )
			DISPOSE( sysdata.time_of_max );
		snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
		sysdata.time_of_max = str_dup( buf );
		sysdata.alltimemax = sysdata.maxplayers;
		snprintf( log_buf, MAX_STRING_LENGTH, "Broke all-time maximum player record: %d", sysdata.alltimemax );
		log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
		to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_STAFF );
		snprintf( buf, MAX_STRING_LENGTH, "&GB&gr&co&gk&Ge &YT&Ohe &RA&rll &CT&ci&Cm&ce &BP&bl&zay&be&Br &PM&pa&Px &GW&gi&Gt&gh &W%d&R!&r!&p!",
			sysdata.alltimemax );
		info_chan( buf );
		save_sysdata( sysdata );
	}
	set_alarm( 0 );
}

void free_desc( DESCRIPTOR_DATA *d )
{
	compressEnd( d );
	close( d->descriptor );
	STRFREE( d->host );
	DISPOSE( d->outbuf );
	if( d->pagebuf )
		DISPOSE( d->pagebuf );
	DISPOSE( d->mccp );
	DISPOSE( d );
	--num_descriptors;
}

void close_socket( DESCRIPTOR_DATA *dclose, bool force )
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d;
	bool DoNotUnlink = FALSE;

	if( dclose->ipid != -1 )
	{
		int status;

		kill( dclose->ipid, SIGKILL );
		waitpid( dclose->ipid, &status, 0 );
	}
	if( dclose->ifd != -1 )
		close( dclose->ifd );

	/*
	 * flush outbuf
	 */
	if( !force && dclose->outtop > 0 )
		flush_buffer( dclose, FALSE );

	/*
	 * say bye to whoever's snooping this descriptor
	 */
	if( dclose->snoop_by )
		write_to_buffer( dclose->snoop_by, "Your victim has left the game.\r\n", 0 );

	/*
	 * stop snooping everyone else
	 */
	for( d = first_descriptor; d; d = d->next )
		if( d->snoop_by == dclose )
			d->snoop_by = NULL;

	/*
	 * Check for switched people who go link-dead. -- Altrag
	 */
	if( dclose->original )
	{
		if( ( ch = dclose->character ) != NULL )
			do_return( ch, "" );
		else
		{
			bug( "%s: dclose->original without character %s", __func__,
				( dclose->original->name ? dclose->original->name : "unknown" ) );
			dclose->character = dclose->original;
			dclose->original = NULL;
		}
	}

	ch = dclose->character;

	/*
	 * sanity check :(
	 */
	if( !dclose->prev && dclose != first_descriptor )
	{
		DESCRIPTOR_DATA *dp, *dn;
		bug( "%s: %s desc:%p != first_desc:%p and desc->prev = NULL!", __func__,
			ch ? ch->name : d->host, dclose, first_descriptor );
		dp = NULL;
		for( d = first_descriptor; d; d = dn )
		{
			dn = d->next;
			if( d == dclose )
			{
				bug( "%s: %s desc:%p found, prev should be:%p, fixing.", __func__, ch ? ch->name : d->host, dclose, dp );
				dclose->prev = dp;
				break;
			}
			dp = d;
		}
		if( !dclose->prev )
		{
			bug( "%s: %s desc:%p could not be found!.", __func__, ch ? ch->name : dclose->host, dclose );
			DoNotUnlink = TRUE;
		}
	}
	if( !dclose->next && dclose != last_descriptor )
	{
		DESCRIPTOR_DATA *dp, *dn;
		bug( "%s: %s desc:%p != last_desc:%p and desc->next = NULL!", __func__,
			ch ? ch->name : d->host, dclose, last_descriptor );
		dn = NULL;
		for( d = last_descriptor; d; d = dp )
		{
			dp = d->prev;
			if( d == dclose )
			{
				bug( "%s: %s desc:%p found, next should be:%p, fixing.", __func__, ch ? ch->name : d->host, dclose, dn );
				dclose->next = dn;
				break;
			}
			dn = d;
		}
		if( !dclose->next )
		{
			bug( "%s: %s desc:%p could not be found!.", __func__, ch ? ch->name : dclose->host, dclose );
			DoNotUnlink = TRUE;
		}
	}

	if( dclose->character )
	{
		snprintf( log_buf, MAX_STRING_LENGTH, "Closing link to %s.", ch->name );
		log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->top_level ) );
		/*
			if ( ch->top_level < LEVEL_LIAISON )
			  to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
		*/
		if( dclose->connected == CON_PLAYING || dclose->connected == CON_EDITING )
		{
			act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
			ch->desc = NULL;
		}
		else
		{
			/*
			 * clear descriptor pointer to get rid of bug message in log
			 */
			dclose->character->desc = NULL;
			free_char( dclose->character );
		}
	}

	if( !DoNotUnlink )
	{
		/*
		 * make sure loop doesn't get messed up
		 */
		if( d_next == dclose )
			d_next = d_next->next;
		UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
	}

	compressEnd( dclose );

	if( dclose->descriptor == maxdesc )
		--maxdesc;

	free_desc( dclose );
}

bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
	size_t iStart;

	/*
	 * Hold horses if pending command already.
	 */
	if( d->incomm[0] != '\0' )
		return TRUE;

	/*
	 * Check for overflow.
	 */
	iStart = strlen( d->inbuf );
	if( iStart >= sizeof( d->inbuf ) - 10 )
	{
		snprintf( log_buf, MAX_STRING_LENGTH, "%s input overflow!", d->host );
		log_string( log_buf );
		write_to_descriptor( d, "\r\n*** PUT A LID ON IT!!! ***\r\n", 0 );
		return FALSE;
	}

	for( ;; )
	{
		int nRead;

		nRead = read( d->descriptor, d->inbuf + iStart, sizeof( d->inbuf ) - 10 - iStart );
		if( nRead > 0 )
		{
			iStart += nRead;
			if( d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r' )
				break;
		}
		else if( nRead == 0 )
		{
			log_string_plus( "EOF encountered on read.", LOG_COMM, sysdata.log_level );
			return FALSE;
		}
		else if( errno == EWOULDBLOCK )
			break;
		else
		{
			perror( "Read_from_descriptor" );
			return FALSE;
		}
	}

	d->inbuf[iStart] = '\0';
	return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
	int i, j, k, iac = 0;

	/*
	 * Hold horses if pending command already.
	 */
	if( d->incomm[0] != '\0' )
		return;

	/*
	 * Look for at least one new line.
	 */
	for( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i < MAX_INBUF_SIZE; i++ )
	{
		if( d->inbuf[i] == '\0' )
			return;
	}

	/*
	 * Canonical input processing.
	 */
	for( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
	{
		if( k >= 254 )
		{
			write_to_descriptor( d, "Line too long.\r\n", 0 );

			/*
			 * skip the rest of the line
			 */
			 /*
			  * for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
			  * {
			  * if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
			  * break;
			  * }
			  */
			d->inbuf[i] = '\n';
			d->inbuf[i + 1] = '\0';
			break;
		}

		if( d->inbuf[i] == ( signed char ) IAC )
			iac = 1;
		else if( iac == 1
			&& ( d->inbuf[i] == ( signed char ) DO || d->inbuf[i] == ( signed char ) DONT
				|| d->inbuf[i] == ( signed char ) WILL ) )
			iac = 2;
		else if( iac == 2 )
		{
			iac = 0;
			if( d->inbuf[i] == ( signed char ) TELOPT_COMPRESS2 )
			{
				if( d->inbuf[i - 1] == ( signed char ) DO )
					compressStart( d );
				else if( d->inbuf[i - 1] == ( signed char ) DONT )
					compressEnd( d );
			}
		}
		else if( d->inbuf[i] == '\b' && k > 0 )
			--k;
		else if( isascii( d->inbuf[i] ) && isprint( d->inbuf[i] ) )
			d->incomm[k++] = d->inbuf[i];
	}

	/*
	 * Finish off the line.
	 */
	if( k == 0 )
		d->incomm[k++] = ' ';
	d->incomm[k] = '\0';

	/*
	 * Deal with bozos with #repeat 1000 ...
	 */
	if( k > 1 || d->incomm[0] == '!' )
	{
		if( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
		{
			d->repeat = 0;
		}
		else
		{
			if( ++d->repeat >= 20 )
			{
				/*		snprintf( log_buf, MAX_STRING_LENGTH, "%s input spamming!", d->host );
						log_string( log_buf );
				*/
				write_to_descriptor( d, "\r\n*** PUT A LID ON IT!!! ***\r\n", 0 );
			}
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if( d->incomm[0] == '!' )
		mudstrlcpy( d->incomm, d->inlast, MAX_INPUT_LENGTH );
	else
		mudstrlcpy( d->inlast, d->incomm, MAX_INPUT_LENGTH );

	/*
	 * Shift the input buffer.
	 */
	while( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		i++;
	for( j = 0; ( d->inbuf[j] = d->inbuf[i + j] ) != '\0'; j++ )
		;
}

/*
 * Low level output function.
 */
bool flush_buffer( DESCRIPTOR_DATA *d, bool fPrompt )
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;

	ch = d->original ? d->original : d->character;
	if( ch && ch->fighting && ch->fighting->who )
		show_condition( ch, ch->fighting->who );

	/*
	 * If buffer has more than 4K inside, spit out .5K at a time   -Thoric
	 */
	if( !mud_down && d->outtop > 4096 )
	{
		memcpy( buf, d->outbuf, 512 );
		memmove( d->outbuf, d->outbuf + 512, d->outtop - 512 );
		d->outtop -= 512;

		if( d->snoop_by )
		{
			char snoopbuf[MAX_INPUT_LENGTH];

			buf[512] = '\0';
			if( d->character && d->character->name )
			{
				if( d->original && d->original->name )
					snprintf( snoopbuf, MAX_INPUT_LENGTH, "%s (%s)", d->character->name, d->original->name );
				else
					snprintf( snoopbuf, MAX_INPUT_LENGTH, "%s", d->character->name );
				write_to_buffer( d->snoop_by, snoopbuf, 0 );
			}
			write_to_buffer( d->snoop_by, "% ", 2 );
			write_to_buffer( d->snoop_by, buf, 0 );
		}
		if( !write_to_descriptor( d, buf, 512 ) )
		{
			d->outtop = 0;
			return FALSE;
		}
		return TRUE;
	}

	/*
	 * Bust a prompt.
	 */
	if( fPrompt && !mud_down && d->connected == CON_PLAYING )
	{
		ch = d->original ? d->original : d->character;
		if( xIS_SET( ch->act, PLR_BLANK ) )
			write_to_buffer( d, "\r\n", 2 );

		if( xIS_SET( ch->act, PLR_PROMPT ) )
			display_prompt( d );
		if( xIS_SET( ch->act, PLR_TELNET_GA ) )
			write_to_buffer( d, ( const char * ) go_ahead_str, 0 );
	}

	/*
	 * Short-circuit if nothing to write.
	 */
	if( d->outtop == 0 )
		return true;

	/*
	 * Snoop-o-rama.
	 */
	if( d->snoop_by )
	{
		/*
		 * without check, 'force mortal quit' while snooped caused crash, -h
		 */
		if( d->character && d->character->name )
		{
			/*
			 * Show original snooped names. -- Altrag
			 */
			if( d->original && d->original->name )
				snprintf( buf, MAX_INPUT_LENGTH, "%s (%s)", d->character->name, d->original->name );
			else
				snprintf( buf, MAX_INPUT_LENGTH, "%s", d->character->name );
			write_to_buffer( d->snoop_by, buf, 0 );
		}
		write_to_buffer( d->snoop_by, "% ", 2 );
		write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
	}

	/*
	 * OS-dependent output.
	 */
	if( !write_to_descriptor( d, d->outbuf, d->outtop ) )
	{
		d->outtop = 0;
		return false;
	}
	else
	{
		d->outtop = 0;
		return true;
	}
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, size_t length )
{
	if( !d )
	{
		bug( "%s: NULL descriptor", __func__ );
		return;
	}

	/*
	 * Normally a bug... but can happen if loadup is used.
	 */
	if( !d->outbuf )
		return;

	/*
	 * Find length in case caller didn't.
	 */
	if( length <= 0 )
		length = strlen( txt );

	/* Uncomment if debugging or something
		if ( length != strlen(txt) )
		{
		bug( "%s: length(%d) != strlen(txt)!", __func__, length );
		length = strlen(txt);
		}
	*/

	/*
	 * Initial \r\n if needed.
	 */
	if( d->outtop == 0 && !d->fcommand )
	{
		d->outbuf[0] = '\n';
		d->outbuf[1] = '\r';
		d->outtop = 2;
	}

	/*
	 * Expand the buffer as needed.
	 */
	while( d->outtop + length >= d->outsize )
	{
		if( d->outsize > 32000 )
		{
			/*
			 * empty buffer
			 */
			d->outtop = 0;
			bug( "%s: Buffer overflow. Closing (%s).", __func__, d->character ? d->character->name : "???" );
			close_socket( d, true );
			return;
		}
		d->outsize *= 2;
		RECREATE( d->outbuf, char, d->outsize );
	}

	/*
	 * Copy.
	 */
	strncpy( d->outbuf + d->outtop, txt, length );
	d->outtop += length;
	d->outbuf[d->outtop] = '\0';
}

void buffer_printf( DESCRIPTOR_DATA *d, const char *fmt, ... )
{
	char buf[MAX_STRING_LENGTH * 2];

	va_list args;

	va_start( args, fmt );
	vsnprintf( buf, MAX_STRING_LENGTH, fmt, args );
	va_end( args );

	write_to_buffer( d, colorize( buf, d ), strlen( buf ) );
}

/*
* This is the MCCP version. Use write_to_descriptor_old to send non-compressed
* text.
* Updated to run with the block checks by Orion... if it doesn't work, blame
* him.;P -Orion
*/
bool write_to_descriptor( DESCRIPTOR_DATA *d, const char *txt, int length )
{
	int iStart = 0;
	int nWrite = 0;
	int nBlock;
	int iErr;
	int len;

	if( length <= 0 )
		length = strlen( txt );

	if( d && d->mccp->out_compress )
	{
		d->mccp->out_compress->next_in = ( unsigned char * ) txt;
		d->mccp->out_compress->avail_in = length;

		while( d->mccp->out_compress->avail_in )
		{
			d->mccp->out_compress->avail_out =
				COMPRESS_BUF_SIZE - ( d->mccp->out_compress->next_out - d->mccp->out_compress_buf );

			if( d->mccp->out_compress->avail_out )
			{
				int status = deflate( d->mccp->out_compress, Z_SYNC_FLUSH );

				if( status != Z_OK )
					return false;
			}

			len = d->mccp->out_compress->next_out - d->mccp->out_compress_buf;
			if( len > 0 )
			{
				for( iStart = 0; iStart < len; iStart += nWrite )
				{
					nBlock = UMIN( len - iStart, 4096 );
					nWrite = send( d->descriptor, d->mccp->out_compress_buf + iStart, nBlock, 0 );
					if( nWrite == -1 )
					{
						iErr = errno;
						if( iErr == EWOULDBLOCK )
						{
							/*
							 * This is a SPAMMY little bug error. I would suggest
							 * not using it, but I've included it in case. -Orion
							 *
							 perror( "Write_to_descriptor: Send is blocking" );
							 */
							nWrite = 0;
							continue;
						}
						else
						{
							perror( "Write_to_descriptor" );
							return false;
						}
					}

					if( !nWrite )
						break;
				}

				if( !iStart )
					break;

				if( iStart < len )
					memmove( d->mccp->out_compress_buf, d->mccp->out_compress_buf + iStart, len - iStart );

				d->mccp->out_compress->next_out = d->mccp->out_compress_buf + len - iStart;
			}
		}
		return true;
	}

	for( iStart = 0; iStart < length; iStart += nWrite )
	{
		nBlock = UMIN( length - iStart, 4096 );
		nWrite = send( d->descriptor, txt + iStart, nBlock, 0 );
		if( nWrite == -1 )
		{
			iErr = errno;
			if( iErr == EWOULDBLOCK )
			{
				/*
				 * This is a SPAMMY little bug error. I would suggest
				 * not using it, but I've included it in case. -Orion
				 *
				 perror( "Write_to_descriptor: Send is blocking" );
				 */
				nWrite = 0;
				continue;
			}
			else
			{
				perror( "Write_to_descriptor" );
				return false;
			}
		}
	}
	return true;
}

void descriptor_printf( DESCRIPTOR_DATA *d, const char *fmt, ... )
{
	char buf[MAX_STRING_LENGTH * 2];

	va_list args;

	va_start( args, fmt );
	vsnprintf( buf, MAX_STRING_LENGTH, fmt, args );
	va_end( args );

	write_to_descriptor( d, buf, strlen( buf ) );
}

/*
 *
 * Added block checking to prevent random booting of the descriptor. Thanks go
 * out to Rustry for his suggestions. -Orion
 */
bool write_to_descriptor_old( int desc, const char *txt, int length )
{
	int iStart = 0;
	int nWrite = 0;
	int nBlock = 0;
	int iErr = 0;

	if( length <= 0 )
		length = strlen( txt );

	for( iStart = 0; iStart < length; iStart += nWrite )
	{
		nBlock = UMIN( length - iStart, 4096 );
		nWrite = send( desc, txt + iStart, nBlock, 0 );

		if( nWrite == -1 )
		{
			iErr = errno;
			if( iErr == EWOULDBLOCK )
			{
				/*
				 * This is a SPAMMY little bug error. I would suggest
				 * not using it, but I've included it in case. -Orion
				 *
				 perror( "Write_to_descriptor: Send is blocking" );
				 */
				nWrite = 0;
				continue;
			}
			else
			{
				perror( "Write_to_descriptor" );
				return false;
			}
		}
	}
	return true;
}

void show_title( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	if( !IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) )
	{
		if( xIS_SET( ch->act, PLR_ANSI ) )
			send_ansi_title( ch );
		else
			send_ascii_title( ch );
	}
	else
	{
		write_to_buffer( d, "Press enter...\r\n", 0 );
	}
	d->connected = CON_PRESS_ENTER;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, const char *argument )
{
	while( isspace( *argument ) )
		argument++;

	switch( d->connected )
	{
	default:
		bug( "%s: bad d->connected %d.", __func__, d->connected );
		close_socket( d, TRUE );
		return;

	case CON_GET_NAME:
		nanny_get_name( d, argument );
		break;

	case CON_GET_OLD_PASSWORD:
		nanny_get_old_password( d, argument );
		break;

	case CON_CONFIRM_NEW_NAME:
		nanny_confirm_new_name( d, argument );
		break;

	case CON_GET_NEW_PASSWORD:
		nanny_get_new_password( d, argument );
		break;

	case CON_CONFIRM_NEW_PASSWORD:
		nanny_confirm_new_password( d, argument );
		break;

	case CON_AGREEMENT:
		nanny_confirm_agreement( d, argument );
		break;

	case CON_GET_NEW_SEX:
		nanny_get_new_sex( d, argument );
		break;

	case CON_GET_NEW_RACE:
		nanny_get_new_race( d, argument );
		break;

	case CON_GET_HAIR:
		nanny_get_hair( d, argument );
		break;

	case CON_GET_HIGHLIGHT:
		nanny_get_highlights( d, argument );
		break;

	case CON_GET_EYE:
		nanny_get_eye_color( d, argument );
		break;

	case CON_GET_BUILD:
		nanny_get_build( d, argument );
		break;

	case CON_GET_HERO:
		nanny_get_hero( d, argument );
		break;

	case CON_GET_NEW_CLASS:
		nanny_get_new_class( d, argument );
		break;

	case CON_ROLL_STATS:
		nanny_roll_stats( d, argument );
		break;

	case CON_STATS_OK:
		nanny_stats_ok( d, argument );
		break;

	case CON_PRESS_ENTER:
		nanny_press_enter( d, argument );
		break;

	case CON_WANT_INVIS:
		nanny_get_invis( d, argument );
		break;

	case CON_READ_MOTD:
		nanny_read_motd( d, argument );
		break;

	case CON_NOTE_TO:
		handle_con_note_to( d, argument );
		break;

	case CON_NOTE_SUBJECT:
		handle_con_note_subject( d, argument );
		break;

	case CON_NOTE_EXPIRE:
		handle_con_note_expire( d, argument );
		break;

	case CON_NOTE_TEXT:
		handle_con_note_text( d, argument );
		break;

	case CON_NOTE_FINISH:
		handle_con_note_finish( d, argument );
		break;
	}
}

void nanny_get_name( DESCRIPTOR_DATA *d, const char *orig_argument )
{
	CHAR_DATA *ch;
	BAN_DATA *pban;
	char buf[MSL];
	bool fOld;
	short chk;
	
	if( orig_argument[0] == '\0' )
	{
		close_socket( d, FALSE );
		return;
	}

	if( !str_cmp( orig_argument, "who" ) )
	{
		do_who( NULL, "" );
		show_file_to_desc( d, WHO_FILE );
		send_to_desc_color( "&CName&c: ", d );
		return;
	}

	char argument[MAX_STRING_LENGTH - 30];
	mudstrlcpy( argument, orig_argument, MAX_STRING_LENGTH - 30 );
	argument[0] = UPPER( argument[0] );

	if( !check_parse_name( argument ) )
	{
		send_to_desc_color( "&BIllegal name&b,&B try another&b!\r\n&CName&c: ", d );
		return;
	}

	if( !str_cmp( argument, "New" ) )
	{
		if( d->newstate == 0 )
		{
			/*
			 * New player
			 */
			 /*
			  * Don't allow new players if DENY_NEW_PLAYERS is true
			  */
			if( sysdata.DENY_NEW_PLAYERS == true )
			{
				write_to_buffer( d, "The mud is currently preparing for a reboot.\r\n", 0 );
				write_to_buffer( d, "New players are not accepted during this time.\r\n", 0 );
				write_to_buffer( d, "Please try again in a few minutes.\r\n", 0 );
				close_socket( d, false );
			}
			write_to_buffer( d, "\r\nChoosing a name is one of the most important parts of this game...\r\n"
							"Make sure to pick a name appropriate to the character you are going\r\n"
							"to role play, and be sure that it suits our theme.\r\n"
							"If the name you select is not acceptable, you will be asked to choose\r\n"
							"another one.\r\n\r\nPlease choose a name for your character: ", 0 );
			d->newstate++;
			d->connected = CON_GET_NAME;
			return;
		}
		else
		{
			send_to_desc_color( "&BIllegal name&b,&B try another&b!\r\n&CName&c: ", d );
			return;
		}
	}

	if( check_playing( d, argument, false ) == BERR )
	{
		send_to_desc_color( "&CName&c: ", d );
		return;
	}

	fOld = load_char_obj( d, argument, true, false );
	if( !d->character )
	{
		snprintf( log_buf, MAX_STRING_LENGTH, "Bad player file %s@%s.", argument, d->host );
		log_string( log_buf );
		write_to_buffer( d, "Your playerfile is corrupt...Please notify the admins.\r\n", 0 );
		close_socket( d, false );
		return;
	}
	ch = d->character;

	for( pban = first_ban; pban; pban = pban->next )
	{
		if( ( !str_prefix( pban->name, d->host )
			&& pban->level >= ch->top_level )
			|| ( !str_suffix( pban->name, d->host ) && pban->level >= ch->top_level ) )
		{
			send_to_desc_color( "&PYour site has been &r-&R=&PBa&pnn&Ped&R=&r-\r\n", d );
			close_socket( d, false );
			return;
		}
	}
	if( xIS_SET( ch->act, PLR_DENY ) )
	{
		snprintf( log_buf, MAX_STRING_LENGTH, "Denying access to %s@%s.", argument, d->host );
		log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
		if( d->newstate != 0 )
		{
			send_to_desc_color( "&CThat name is already &RTaken&c.&C  Please choose another&c: ", d );
			d->connected = CON_GET_NAME;
			d->character->desc = NULL;
			free_char( d->character );
			d->character = NULL;
			return;
		}
		send_to_desc_color( "&CThis character has been &RD&rE&RN&rI&RE&rD&C!\r\n", d );
		close_socket( d, false );
		return;
	}

	if( IS_IMMORTAL( ch ) && !check_immortal_domain( ch, d->host ) )
	{
		snprintf( log_buf, MAX_STRING_LENGTH, "%s's Immortal hacking attempt from %s.", ch->name, d->host );
		log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
		write_to_buffer( d, "You shouldn't try logging on a character that isn't yours..\r\n", 0 );
		close_socket( d, false );
		return;
	}
	
	chk = check_reconnect( d, argument, false );
	if( chk == BERR )
		return;

	if( chk )
	{
		fOld = true;
	}
	else
	{
		if( wizlock && !IS_IMMORTAL( ch ) )
		{
			write_to_buffer( d, "The game is wizlocked.  Only immortals can connect now.\r\n", 0 );
			write_to_buffer( d, "Please try back later.\r\n", 0 );
			close_socket( d, false );
			return;
		}
	}

	if( fOld )
	{
		if( d->newstate != 0 )
		{
			send_to_desc_color( "&CThat name is already &RTaken&c.&C  Please choose another&b: ", d );
			d->connected = CON_GET_NAME;
			d->character->desc = NULL;
			free_char( d->character );
			d->character = NULL;
			return;
		}
		/*
		 * Old player
		 */
		send_to_desc_color( "&zPassword&B&W: ", d );
		write_to_buffer( d, ( const char * ) echo_off_str, 0 );
		d->connected = CON_GET_OLD_PASSWORD;
		return;
	}
	else
	{
		send_to_desc_color( "\r\n&CThat name isn't in our Database&c, &Cyou must be new here&c!\r\n\r\n", d );
		snprintf( buf, MAX_STRING_LENGTH*2, "&zIs &g'&G%s&g' &zyour name&B&W? &c(&YY&c/&YN&c)&B? \r\n", argument );
		send_to_desc_color( buf, d );
		d->connected = CON_CONFIRM_NEW_NAME;
		return;
	}

}

void nanny_get_old_password( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MSL];
	bool fOld;
	short chk;

	write_to_buffer( d, "\r\n", 2 );
	if( strcmp( sha256_crypt( argument ), ch->pcdata->pwd ) )
	{
		write_to_buffer( d, "Wrong password, disconnecting.\r\n", 0 );
		/*
		 * clear descriptor pointer to get rid of bug message in log
		 */
		d->character->desc = NULL;
		close_socket( d, false );
		return;
	}
	write_to_buffer( d, ( const char * ) echo_on_str, 0 );

	if( check_playing( d, ch->name, true ) )
		return;

	chk = check_reconnect( d, ch->name, true );
	if( chk == BERR )
	{
		if( d->character && d->character->desc )
			d->character->desc = NULL;
		close_socket( d, false );
		return;
	}
	if( chk == true )
		return;

	if( check_multi( d, ch->name ) )
	{
		close_socket( d, false );
		return;
	}
	mudstrlcpy( buf, ch->name, MAX_STRING_LENGTH );
	d->character->desc = NULL;
	free_char( d->character );
	fOld = load_char_obj( d, buf, false, false );
	if( !fOld )
		bug( "%s: failed to load_char_obj for %s.", __func__, buf );

	if( !d->character )
	{
		log_printf( "Bad player file %s@%s.", argument, d->host );
		write_to_buffer( d, "Your playerfile is corrupt... Please notify the admins.\r\n", 0 );
		close_socket( d, FALSE );
		return;
	}
	ch = d->character;
	log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->top_level ), "%s (%s) has connected.", ch->name, d->host );
	show_title( d );
	{
		struct tm *tme;
		time_t now;
		char day[50];
		now = time( 0 );
		tme = localtime( &now );
		strftime( day, 50, "%a %b %d %H:%M:%S %Y", tme );
		snprintf( log_buf, MAX_STRING_LENGTH, "%-20s     %-24s    %s", ch->name, day, d->host );
		write_last_file( log_buf );
	}
	//if( ch->pcdata->area )
	//	do_loadarea( ch, "" );

}

void nanny_confirm_new_name( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MSL];

	switch( *argument )
	{
	case 'y':
	case 'Y':
		snprintf( buf, 256, "../storage/%s", capitalize( ch->name ) );

		if( !remove( buf ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "Leftover storage found and removed for %s.", ch->name );
			log_string( buf );
		}
		snprintf( buf, MAX_STRING_LENGTH, "\r\n&CPlease pick a password&c, &Can alphanumeric Password would be best&c."
			"\r\n&zPick a password for &G%s&B&W: %s", capitalize( ch->name ), ( const char * ) echo_off_str );
		send_to_desc_color( buf, d );
		d->connected = CON_GET_NEW_PASSWORD;
		break;

	case 'n':
	case 'N':
		send_to_desc_color( "&zThen what &B>&RIS&B< &zyour name&B&W? ", d );
		/*
		 * clear descriptor pointer to get rid of bug message in log
		 */
		d->character->desc = NULL;
		free_char( d->character );
		d->character = NULL;
		d->connected = CON_GET_NAME;
		break;

	default:
		send_to_desc_color( "&CPlease type &cYes &Cor &cNo&C. ", d );
		break;
	}

}

void nanny_get_new_password( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char *pwdnew;

	write_to_buffer( d, "\r\n", 2 );

	if( strlen( argument ) < 5 )
	{
		send_to_desc_color( "&CPasswords must be atleast &c5&C characters long&c.\r\n&zPassword&B&W: ", d );
		return;
	}

	pwdnew = sha256_crypt( argument ); /* SHA-256 Encryption */

	DISPOSE( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( pwdnew );
	send_to_desc_color( "\r\n&zPlease retype your password to confirm&B&W: ", d );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
}

void nanny_confirm_new_password( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	write_to_buffer( d, "\r\n", 2 );

	if( strcmp( sha256_crypt( argument ), ch->pcdata->pwd ) )
	{
		send_to_desc_color( "&CPasswords &Rdon't&C match&c.\r\n&zRetype Password&B&W: ", d );
		d->connected = CON_GET_NEW_PASSWORD;
		return;
	}
	write_to_buffer( d, ( const char * ) echo_on_str, 0 );
	send_to_desc_color( "\r\n&RAttention&r:\r\n", d );
	send_to_desc_color( "\r\n&BYou come here to play on our MUD. We don't force you to\n"
		"play here. What we say goes. You must follow the rules and\n"
		"listen to what the Immortals say. IF you don't obey the rules\n"
		"you give up your right to play here, and anything you may have\n"
		"earned while you were here.\r\n\r\n"
		"          This MUD is for adult audiences only.\n"
		"        You must be atleast 14 years old to play.\n"
		"This is a timeline in Gundam Wing that COULD have happened.\n"
		"Remember, the immortals of this MUD work hard so you can have\n"
		"a nice place to play. So do not under any circumstance mock\n" "or disrespect them.\r\n", d );
	send_to_desc_color( "\r\n&zDo you &GAgree &zor &RDisagree&z to these terms?\r\n", d );
	d->connected = CON_AGREEMENT;
}

void nanny_confirm_agreement( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;

	switch( argument[0] )
	{
	case 'a':
	case 'A':
		send_to_desc_color( "&CThank you.\r\n", d );
		break;
	case 'd':
	case 'D':
		send_to_desc_color( "\r\n\r\n&CSorry you don't agree. Come back when you can.\r\n", d );
		snprintf( log_buf, MAX_STRING_LENGTH, "%s at %s didn't accept the agreement.", ch->name, d->host );
		log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
		d->character->desc = NULL;
		close_socket( d, false );
		return;
		break;
	default:
		send_to_desc_color( "\r\n&zThat's &RNOT&z an option&W.\r\n\r\n&zPlease &GAGREE&z or &RDISAGREE&W! ", d );
		return;
	}
	write_to_buffer( d, ( const char * ) echo_on_str, 0 );
	send_to_desc_color( "\r\n&CWhat gender would you like&c, &c(&BM&bale&c/&PF&pemale&c/&WN&wonbinary&c)&C? ", d );
	d->connected = CON_GET_NEW_SEX;
}

void nanny_get_new_sex( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MSL];
	int iRace;

	switch( argument[0] )
	{
	case 'm':
	case 'M':
		ch->sex = SEX_MALE;
		break;
	case 'f':
	case 'F':
		ch->sex = SEX_FEMALE;
		break;
	case 'n':
	case 'N':
		ch->sex = SEX_NONBINARY;
		break;
	default:
		send_to_desc_color( "&CThat's &Rnot&z a gender&c.\r\n&CPlease choose &c(&BM&bale&c/&PF&pemale&c/&WN&wonbinary&c)&C! ", d );
		return;
	}


	send_to_desc_color( "\r\n\r\n&zPlease choose a Nationality&B&W:\r\n&B[&R", d );
	buf[0] = '\0';
	for( iRace = 0; iRace < 17; iRace++ )
	{
		if( race_table[iRace].race_name && race_table[iRace].race_name[0] != '\0' )
		{
			if( iRace > 0 )
			{
				if( strlen( buf ) + strlen( race_table[iRace].race_name ) > 77 )
				{
					mudstrlcat( buf, "\r\n", MSL );
					write_to_buffer( d, buf, 0 );
					buf[0] = '\0';
				}
				else
					mudstrlcat( buf, " ", MSL );
			}
			mudstrlcat( buf, race_table[iRace].race_name, MSL );
		}
	}
	mudstrlcat( buf, "&B]\r\n: ", MSL );
	send_to_desc_color( buf, d );
	d->connected = CON_GET_NEW_RACE;

}

void nanny_get_new_race( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char arg[MSL], buf[MSL], buf2[MSL];
	int iRace, col, iHair;

	argument = one_argument( argument, arg );
	if( !str_cmp( arg, "help" ) )
	{
		do_help( ch, argument );
		send_to_desc_color( "&zPlease choose a Nationality&B&W: ", d );
		return;
	}


	for( iRace = 0; iRace < MAX_RACE; iRace++ )
	{
		if( toupper( arg[0] ) == toupper( race_table[iRace].race_name[0] )
			&& !str_prefix( arg, race_table[iRace].race_name ) )
		{
			ch->race = iRace;
			break;
		}
	}

	if( iRace == MAX_RACE || !race_table[iRace].race_name || race_table[iRace].race_name[0] == '\0' )
	{
		send_to_desc_color( "&CThat &Risn't&C a Nationality&c.\r\n&zWhat &B>&RIS&B< &zyour Nationality&B&W? ", d );
		return;
	}

	send_to_desc_color( "\r\n\r\n&CChoose a hair color&B&W:\r\n", d );
	buf[0] = '\0';
	col = 0;
	for( iHair = 0; iHair <= 22; iHair++ )
	{
		if( hair_list[iHair] && hair_list[iHair][0] != '\0' )
		{
			if( iHair > 0 )
			{
				if( ++col % 3 == 0 )
				{
					strcat( buf, "\r\n" );
					send_to_desc_color( buf, d );
					buf[0] = '\0';
				}
				else
					strcat( buf, "       " );
			}
			snprintf( buf2, MAX_STRING_LENGTH, "&B[&R%-2d&B]&R ", iHair );
			strcat( buf, buf2 );
			snprintf( buf2, MAX_STRING_LENGTH, "%-16.16s", hair_list[iHair] );
			strcat( buf, buf2 );
		}
	}
	strcat( buf, "\r\n: " );
	send_to_desc_color( buf, d );
	d->connected = CON_GET_HAIR;

}

void nanny_get_hair( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char arg[MSL], buf[MSL], buf2[MSL];
	int iHair, col, iHighlight;

	argument = one_argument( argument, arg );
	for( iHair = 0; iHair < 22; iHair++ )
	{
		if( toupper( arg[0] ) == toupper( hair_list[iHair][0] ) && !str_prefix( arg, hair_list[iHair] ) )
		{
			ch->pcdata->hair = iHair;
			break;
		}
	}
	if( iHair == 23 || str_cmp( arg, hair_list[iHair] ) || hair_list[iHair][0] == '\0' )
	{
		send_to_desc_color( "&CThat &Risn't&C a hair color&c.\r\n&CWhat &B>&RIS&B<&C it going to be? ", d );
		return;
	}

	send_to_desc_color( "\r\n\r\n&CChoose from the following Highlights&c.\r\n", d );
	buf[0] = '\0';
	col = 0;
	for( iHighlight = 0; iHighlight <= 22; iHighlight++ )
	{
		if( highlight_list[iHighlight] && highlight_list[iHighlight][0] != '\0' )
		{
			if( iHighlight > 0 )
			{
				if( ++col % 3 == 0 )
				{
					mudstrlcat( buf, "\r\n", MSL );
					send_to_desc_color( buf, d );
					buf[0] = '\0';
				}
				else
					mudstrlcat( buf, "       ", MSL );
			}
			snprintf( buf2, MAX_STRING_LENGTH, "&B[&R%-2d&B]&R ", iHighlight );
			mudstrlcat( buf, buf2, MSL );
			snprintf( buf2, MAX_STRING_LENGTH, "%-16.16s", highlight_list[iHighlight] );
			mudstrlcat( buf, buf2, MSL );
		}
	}
	mudstrlcat( buf, "\r\n: ", MSL );
	send_to_desc_color( buf, d );
	d->connected = CON_GET_HIGHLIGHT;
}

void nanny_get_highlights( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char arg[MSL], buf[MSL], buf2[MSL];
	int iEye, col, iHighlight;

	argument = one_argument( argument, arg );
	for( iHighlight = 0; iHighlight < 22; iHighlight++ )
	{
		if( toupper( arg[0] ) == toupper( highlight_list[iHighlight][0] )
			&& !str_prefix( arg, highlight_list[iHighlight] ) )
		{
			ch->pcdata->highlight = iHighlight;
			break;
		}
	}
	if( iHighlight == 23 || str_cmp( arg, highlight_list[iHighlight] ) || highlight_list[iHighlight][0] == '\0' )
	{
		send_to_desc_color( "&CThat isn't a Highlight.\r\n&CWhat &B>&RIS&B<&C it going to be? ", d );
		return;
	}

	send_to_desc_color( "\r\n\r\n&CPlease choose from the following eye colors.\r\n", d );
	buf[0] = '\0';
	col = 0;
	for( iEye = 0; iEye <= 15; iEye++ )
	{
		if( eye_list[iEye] && eye_list[iEye][0] != '\0' )
		{
			if( iEye > 0 )
			{
				if( ++col % 3 == 0 )
				{
					mudstrlcat( buf, "\r\n", MSL );
					send_to_desc_color( buf, d );
					buf[0] = '\0';
				}
				else
					mudstrlcat( buf, "       ", MSL );
			}
			snprintf( buf2, MAX_STRING_LENGTH, "&B[&R%-2d&B]&R ", iEye );
			mudstrlcat( buf, buf2, MSL );
			snprintf( buf2, MAX_STRING_LENGTH, "%-16.16s", eye_list[iEye] );
			mudstrlcat( buf, buf2, MSL );
		}
	}
	mudstrlcat( buf, "\r\n: ", MSL );
	send_to_desc_color( buf, d );
	d->connected = CON_GET_EYE;
}

void nanny_get_eye_color( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char arg[MSL], buf[MSL], buf2[MSL];
	int iEye, col, iBuild;

	argument = one_argument( argument, arg );
	for( iEye = 0; iEye < 15; iEye++ )
	{
		if( toupper( arg[0] ) == toupper( eye_list[iEye][0] ) && !str_prefix( arg, eye_list[iEye] ) )
		{
			ch->pcdata->eye = iEye;
			break;
		}
	}
	if( iEye == 16 || str_cmp( arg, eye_list[iEye] ) || eye_list[iEye][0] == '\0' )
	{
		send_to_desc_color( "&CThat &Risn't&C an eye color&c.\r\n&CWhat &B>&RIS&B<&C it going to be? ", d );
		return;
	}

	send_to_desc_color( "\r\n\r\n&CPlease choose from the following Builds&c.\r\n", d );
	buf[0] = '\0';
	col = 0;
	for( iBuild = 0; iBuild <= 8; iBuild++ )
	{
		if( build_list[iBuild] && build_list[iBuild][0] != '\0' )
		{
			if( iBuild > 0 )
			{
				if( ++col % 3 == 0 )
				{
					mudstrlcat( buf, "\r\n", MSL );
					send_to_desc_color( buf, d );
					buf[0] = '\0';
				}
				else
					mudstrlcat( buf, "       ", MSL );
			}
			snprintf( buf2, MAX_STRING_LENGTH, "&B[&R%-2d&B]&R ", iBuild );
			mudstrlcat( buf, buf2, MSL );
			snprintf( buf2, MAX_STRING_LENGTH, "%-16.16s", build_list[iBuild] );
			mudstrlcat( buf, buf2, MSL );
		}
	}
	mudstrlcat( buf, "\r\n: ", MSL );
	send_to_desc_color( buf, d );
	d->connected = CON_GET_BUILD;
}

void nanny_get_build( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char arg[MSL], buf[MSL], buf2[MSL];
	int iHero, col, iBuild;

	argument = one_argument( argument, arg );
	for( iBuild = 0; iBuild < 8; iBuild++ )
	{
		if( toupper( arg[0] ) == toupper( build_list[iBuild][0] ) && !str_prefix( arg, build_list[iBuild] ) )
		{
			ch->pcdata->build = iBuild;
			break;
		}
	}
	if( iBuild == 9 || str_cmp( arg, build_list[iBuild] ) || build_list[iBuild][0] == '\0' )
	{
		send_to_desc_color( "&CThat &Risn't&C a Build&c!\r\n&CWhat &B>&RIS&B<&C it going to be&c? ", d );
		return;
	}

	send_to_desc_color( "\r\n\r\n&CPlease choose from the following Hero's.\r\n", d );
	buf[0] = '\0';
	col = 0;
	for( iHero = 0; iHero <= 11; iHero++ )
	{
		if( hero_list[iHero] && hero_list[iHero][0] != '\0' )
		{
			if( iHero > 0 )
			{
				if( ++col % 3 == 0 )
				{
					mudstrlcat( buf, "\r\n", MSL );
					send_to_desc_color( buf, d );
					buf[0] = '\0';
				}
				else
					mudstrlcat( buf, "       ", MSL );
			}
			snprintf( buf2, MAX_STRING_LENGTH, "&B[&R%-2d&B]&R ", iHero );
			mudstrlcat( buf, buf2, MSL );
			snprintf( buf2, MAX_STRING_LENGTH, "%-16.16s", hero_list[iHero] );
			mudstrlcat( buf, buf2, MSL );
		}
	}
	mudstrlcat( buf, "\r\n: ", MSL );
	send_to_desc_color( buf, d );
	d->connected = CON_GET_HERO;
}

void nanny_get_hero( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char arg[MSL], buf[MSL], buf2[MSL];
	int iHero, col, iClass;

	argument = one_argument( argument, arg );
	for( iHero = 0; iHero < 11; iHero++ )
	{
		if( toupper( arg[0] ) == toupper( hero_list[iHero][0] ) && !str_prefix( arg, hero_list[iHero] ) )
		{
			ch->pcdata->hero = iHero;
			break;
		}
	}
	if( iHero == 12 || str_cmp( arg, hero_list[iHero] ) || hero_list[iHero][0] == '\0' )
	{
		send_to_desc_color( "&CThat &Risn't&C a Hero&c.\r\n&CWhat &B>&RIS&B<&C it going to be&c? ", d );
		return;
	}
	send_to_desc_color( "\r\n\r\n&CPlease choose a main ability from the following classes&B&W:\r\n", d );
	buf[0] = '\0';
	col = 0;
	for( iClass = 0; iClass < MAX_ABILITY - 1; iClass++ )
	{
		if( ability_name[iClass] && ability_name[iClass][0] != '\0' )
		{
			if( iClass > 0 )
			{
				if( color_strlen( buf ) + strlen( ability_name[iClass] ) > 77 )
				{
					mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
					send_to_desc_color( buf, d );
					buf[0] = '\0';
				}
				else
					mudstrlcat( buf, "", MAX_STRING_LENGTH );
			}
			mudstrlcat( buf, "&B[&R", MSL );
			snprintf( buf2, MAX_STRING_LENGTH, "%-18.18s", ability_name[iClass] );
			mudstrlcat( buf, buf2, MSL );
			mudstrlcat( buf, "&B]", MSL );
			if( ++col % 3 == 0 )
			{
				strcat( buf, "\r\n" );
				send_to_desc_color( buf, d );
				buf[0] = '\0';
			}
		}
	}
	mudstrlcat( buf, "\r\n:", MSL );
	send_to_desc_color( buf, d );
	d->connected = CON_GET_NEW_CLASS;

}

void nanny_get_new_class( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char arg[MSL];
	int iClass;

	argument = one_argument( argument, arg );
	if( !str_cmp( arg, "help" ) )
	{
		do_help( ch, argument );
		send_to_desc_color( "&zPlease choose an ability class&B&W: ", d );
		return;
	}
	for( iClass = 0; iClass < MAX_ABILITY; iClass++ )
	{
		if( toupper( arg[0] ) == toupper( ability_name[iClass][0] ) && !str_prefix( arg, ability_name[iClass] ) )
		{
			ch->main_ability = iClass;
			break;
		}
	}
	if( iClass == MAX_ABILITY || !ability_name[iClass] || ability_name[iClass][0] == '\0' )
	{
		send_to_desc_color( "&CThat &Risn't&C a skill class&c.\r\n&CWhat &B>&RIS&B<&C it going to be&c? ", d );
		return;
	}
	send_to_desc_color( "\r\n\r\n&YRolling stats&O....\r\n", d );
	nanny_roll_stats( d, argument );
}

void nanny_roll_stats( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MSL];
	
	name_stamp_stats( ch );
	snprintf( buf, MSL, "\r\n&RSTR&r: &B%d  &RINT&r: &B%d &RWIS&r: &B%d  &RDEX&r:&B %d  &RCON&r:&B %d  &RCHA&r:&B %d&z\r\n",
		ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha );
	send_to_desc_color( buf, d );
	send_to_desc_color( "\r\n&CAre these stats &BOK&C? ", d );
	d->connected = CON_STATS_OK;
}

void nanny_stats_ok( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MSL];

	switch( argument[0] )
	{
	case 'y':
	case 'Y':
		break;
	case 'n':
	case 'N':
		name_stamp_stats( ch );
		snprintf( buf, MSL, "\r\n&RSTR&r: &B%d  &RINT&r: &B%d &RWIS&r: &B%d  &RDEX&r:&B %d  &RCON&r:&B %d  &RCHA&r:&B %d&z\r\n",
			ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha );
		send_to_desc_color( buf, d );
		write_to_buffer( d, "\n\rOK?. ", 0 );
		return;
	default:
		send_to_desc_color( "&GYES &Cor &RNO&C? ", d );
		return;
	}

	xSET_BIT( ch->act, PLR_ANSI );
	snprintf( log_buf, MAX_STRING_LENGTH, "%s@%s new %s.", ch->name, d->host, race_table[ch->race].race_name );
	log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_STAFF );
	write_to_buffer( d, "Press [ENTER] ", 0 );
	show_title( d );
	{
		int ability;

		for( ability = 0; ability < MAX_ABILITY; ability++ )
			ch->skill_level[ability] = 0;
	}
	ch->top_level = 0;
	ch->position = POS_STANDING;
	d->connected = CON_PRESS_ENTER;
}

void nanny_press_enter( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;

	if( xIS_SET( ch->act, PLR_ANSI ) )
		send_to_pager( "\033[2J", ch );
	else
		send_to_pager( "\014", ch );
	if( IS_IMMORTAL( ch ) )
	{
		send_to_pager( "&WImmortal Message of the Day&w\r\n", ch );
		do_help( ch, "imotd" );
	}
	if( ch->top_level > 0 )
	{
		send_to_pager( "\r\n&WMessage of the Day&w\r\n", ch );
		do_help( ch, "motd" );
	}
	if( ch->top_level == 0 )
		do_help( ch, "nmotd" );
	if( IS_IMMORTAL( ch ) && ch->top_level >= 1002 )
	{
		send_to_pager( "\r\n&CWant to login Invis? &RYes&r/&RNo&C, or hit &R[ENTER] &Cto login normally.\r\n", ch );
		d->connected = CON_WANT_INVIS;
	}
	else
	{
		send_to_pager( "\r\n&WPress [ENTER] &Y", ch );
		d->connected = CON_READ_MOTD;
	}
}

void nanny_get_invis( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;

	switch( argument[0] )
	{
	case 'y':
	case 'Y':
		xSET_BIT( ch->act, PLR_WIZINVIS );
		send_to_desc_color( "&GLogging in Invisible.", d );
		break;

	case 'n':
	case 'N':
		break;
	default:
		send_to_desc_color( "&GLogging in normally.\r\n", d );
		break;
	}
	d->connected = CON_READ_MOTD;
}

void nanny_read_motd( DESCRIPTOR_DATA *d, const char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];

	write_to_buffer( d, "\r\n\r\n", 0 );
	add_char( ch );
	d->connected = CON_PLAYING;
	set_char_color( AT_DGREEN, ch );
	if( !xIS_SET( ch->act, PLR_ANSI ) && d->ansi == true )
		d->ansi = false;
	else if( xIS_SET( ch->act, PLR_ANSI ) && d->ansi == false )
		d->ansi = true;
	reset_colors( ch );

	if( ch->top_level == 0 )
	{
		OBJ_DATA *obj;

		ch->pcdata->clan_name = STRALLOC( "" );
		ch->pcdata->clan = NULL;

		ch->perm_lck = number_range( 6, 18 );

		int ability;
		for( ability = 0; ability < MAX_ABILITY; ability++ )
		{
			ch->skill_level[ability] = 1;
			ch->experience[ability] = 0;
		}
		ch->top_level = 1;
		ch->hit = ch->max_hit;
		ch->move = ch->max_move;
		snprintf( buf, MSL, "%s the %s", ch->name, race_table[ch->race].race_name );
		set_title( ch, buf );

		/*
		 * Added by Narn.  Start new characters with autoexit and autgold
		 * already turned on.  Very few people don't use those.
		 */
		xSET_BIT( ch->act, PLR_AUTOGOLD );
		xSET_BIT( ch->act, PLR_AUTOEXIT );
		SET_BIT( ch->deaf, CHANNEL_i104 );
		xSET_BIT( ch->act, PLR_ANSI );
		/*
		 * New players don't have to earn some eq
		 */

		obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_BANNER ), 0 );
		obj_to_char( obj, ch );
		equip_char( ch, obj, WEAR_LIGHT );

		obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_DAGGER ), 0 );
		obj_to_char( obj, ch );
		equip_char( ch, obj, WEAR_WIELD );

		/*
		 * comlink
		 */

		{
			OBJ_INDEX_DATA *obj_ind = get_obj_index( 10424 );
			if( obj_ind != NULL )
			{
				obj = create_object( obj_ind, 0 );
				obj_to_char( obj, ch );
			}
		}

		if( !sysdata.WAIT_FOR_AUTH )
		{
			char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
			ch->pcdata->auth_state = 3;
		}
		else
		{
			char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
			ch->pcdata->auth_state = 1;
			SET_BIT( ch->pcdata->flags, PCFLAG_UNAUTHED );
		}
		/*
		 * Display_prompt interprets blank as default
		 */
		ch->pcdata->prompt = STRALLOC( "" );
	}
	else if( !IS_IMMORTAL( ch ) && ch->pcdata->release_date > current_time )
	{
		char_to_room( ch, get_room_index( 6 ) );
	}
	else if( ch->in_room && !IS_IMMORTAL( ch )
		&& !xIS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT ) && ch->in_room != get_room_index( 6 ) )
	{
		char_to_room( ch, ch->in_room );
	}
	else if( ch->in_room && !IS_IMMORTAL( ch )
		&& xIS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT ) && ch->in_room != get_room_index( 6 ) )
	{
		SHIP_DATA *ship;

		for( ship = first_ship; ship; ship = ship->next )
			if( ch->in_room->vnum >= ship->firstroom && ch->in_room->vnum <= ship->lastroom )
				if( ship->ship_class != SHIP_PLATFORM || ship->starsystem )
					char_to_room( ch, ch->in_room );
	}
	else
	{
		char_to_room( ch, get_room_index( wherehome( ch ) ) );
	}
	if( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
		remove_timer( ch, TIMER_SHOVEDRAG );
	if( get_timer( ch, TIMER_PKILLED ) > 0 )
		remove_timer( ch, TIMER_PKILLED );
	if( ch->plr_home != NULL )
	{
		char filename[256];
		FILE *fph;
		ROOM_INDEX_DATA *storeroom = ch->plr_home;
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		for( obj = storeroom->first_content; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			extract_obj( obj );
		}
		snprintf( filename, sizeof(filename), "%s%c/%s.home", PLAYER_DIR, tolower( ch->name[0] ), capitalize( ch->name ) );
		if( ( fph = FileOpen( filename, "r" ) ) != NULL )
		{
			int iNest;
			OBJ_DATA *tobj, *tobj_next;

			rset_supermob( storeroom );
			for( iNest = 0; iNest < MAX_NEST; iNest++ )
				rgObjNest[iNest] = NULL;
			for( ;; )
			{
				char letter;
				char *word;

				letter = fread_letter( fph );
				if( letter == '*' )
				{
					fread_to_eol( fph );
					continue;
				}
				if( letter != '#' )
				{
					bug( "Load_plr_home: # not found.", 0 );
					bug( ch->name, 0 );
					break;
				}
				word = fread_word( fph );
				if( !str_cmp( word, "OBJECT" ) )  /* Objects  */
					fread_obj( supermob, fph, OS_CARRY );
				else if( !str_cmp( word, "END" ) )    /* Done     */
					break;
				else
				{
					bug( "Load_plr_home: bad section.", 0 );
					bug( ch->name, 0 );
					break;
				}
			}
			FileClose( fph );
			for( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
			{
				tobj_next = tobj->next_content;
				obj_from_char( tobj );
				obj_to_room( tobj, storeroom );
			}
			release_supermob( );
		}
	}
	act( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	if( !IS_IMMORTAL( ch ) )
	{
		CHAR_DATA *vch;

		for( vch = first_char; vch; vch = vch->next )
		{
			if( IS_NPC( vch ) || vch == ch )
				continue;

			if( ch->pcdata->enter && ch->pcdata->enter[0] != '\0' )
			{
				snprintf( buf, MAX_STRING_LENGTH, "&C(&zGW&c:&zOM&C)&w %s", ch->pcdata->enter );

				if( !IS_SET( vch->deaf, CHANNEL_INFO ) )
					act( AT_GOSSIP, buf, ch, NULL, vch, TO_VICT );
			}
			else
			{
				snprintf( buf, MAX_STRING_LENGTH, "&C(&zGW&c:&zOM&C)&w &B%s &RH&ra&Rs &GC&go&Gm&ge &YT&Oo &CP&cl&Ca&cy&z!", ch->name );
				if( !IS_SET( vch->deaf, CHANNEL_INFO ) )
					act( AT_GOSSIP, buf, ch, NULL, vch, TO_VICT );

			}
		}
	}
	if( !IS_NPC( ch ) )
	{
		int chance, chance2, sn;

		for( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
		{
			if( skill_table[sn]->name == NULL )
				break;
			if( skill_table[sn]->guild == ESPIONAGE_ABILITY )
				continue;
			if( ch->pcdata->learned[sn] == 0 )
				continue;
			if( IS_IMMORTAL( ch ) )
				continue;

			chance = number_range( 1, 10 );

			if( chance == 1 )
			{
				chance2 = number_range( 1, 3 );

				if( chance2 == 1 )
				{
					ch->pcdata->learned[sn] -= 1;
				}

				if( chance2 == 2 )
				{
					ch->pcdata->learned[sn] -= 2;
				}

				if( chance2 == 3 )
				{
					ch->pcdata->learned[sn] -= 3;
				}
			}
		}
		save_char_obj( ch );
	}
	if( IS_SET( ch->pcdata->flags, PCFLAG_AUTOWHO ) )
		do_who( ch, "" );
	do_look( ch, "auto" );
	do_unread( ch, "" );
	adjust_hiscore( "pkill", ch, ch->pcdata->pkills );
	adjust_hiscore( "deaths", ch, ch->pcdata->pdeaths );
	adjust_hiscore( "beenfroze", ch, ch->pcdata->beenfroze );
	adjust_hiscore( "hasfroze", ch, ch->pcdata->hasfroze );
	adjust_hiscore( "apkills", ch, ch->pcdata->apkills );
	adjust_hiscore( "apdeaths", ch, ch->pcdata->apdeaths );
	adjust_hiscore( "mkills", ch, ch->pcdata->mkills );
	check_lottery( ch );
	update_storage( ch );
	mail_count( ch );
	if( ch->pcdata->lasthost )
		STRFREE( ch->pcdata->lasthost );
	ch->pcdata->lasthost = STRALLOC( ch->desc->host );
}

bool is_reserved_name( const char *name )
{
	RESERVE_DATA *res;

	for( res = first_reserved; res; res = res->next )
		if( ( *res->name == '*' && !str_infix( res->name + 1, name ) ) || !str_cmp( res->name, name ) )
			return true;
	return false;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( const char *name )
{

	if( is_reserved_name( name ) )
		return false;
	/*
	 * Reserved words.
	 */
	if( is_name
	( name,
		"all auto someone immortal self god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap quit hologram citizen" ) )
		return false;

	/*
	 * Length restrictions.
	 */
	if( strlen( name ) < 3 )
		return false;

	if( strlen( name ) > 12 )
		return false;

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
		const char *pc;
		bool fIll;

		fIll = true;
		for( pc = name; *pc != '\0'; pc++ )
		{
			if( !isalpha( *pc ) )
				return false;
			if( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
				fIll = false;
		}

		if( fIll )
			return false;
	}

	/*
	 * Code that followed here used to prevent players from naming
	 * themselves after mobs... this caused much havoc when new areas
	 * would go in...
	 */

	return true;
}



/*
 * Look for link-dead player to reconnect.
 */
short check_reconnect( DESCRIPTOR_DATA *d, const char *name, bool fConn )
{
	CHAR_DATA *ch;

	for( ch = first_char; ch; ch = ch->next )
	{
		if( !IS_NPC( ch ) && ( !fConn || !ch->desc ) && ch->name && !str_cmp( name, ch->name ) )
		{
			if( fConn && ch->switched )
			{
				write_to_buffer( d, "Already playing.\n\rName: ", 0 );
				d->connected = CON_GET_NAME;
				if( d->character )
				{
					/*
					 * clear descriptor pointer to get rid of bug message in log
					 */
					d->character->desc = NULL;
					free_char( d->character );
					d->character = NULL;
				}
				return BERR;
			}
			if( fConn == false )
			{
				DISPOSE( d->character->pcdata->pwd );
				d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
			}
			else
			{
				/*
				 * clear descriptor pointer to get rid of bug message in log
				 */
				d->character->desc = NULL;
				free_char( d->character );
				d->character = ch;
				ch->desc = d;
				ch->timer = 0;
				send_to_char( "Reconnecting.\r\n", ch );

				if( ch->pcdata->tells >= 1 )
				{
					ch_printf( ch, "\n&GYou have &g%d&G messeges waiting on your CDI.\r&GType &g'&Gcheck&g'&G, to read them.\r\n",
						ch->pcdata->tells );
				}
				act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
				snprintf( log_buf, MAX_STRING_LENGTH, "%s@%s(%s) reconnected.", ch->name, d->host, d->user );
				log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->top_level ) );
				/*
						if ( ch->top_level < LEVEL_LIAISON )
						  to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
				*/
				d->connected = CON_PLAYING;

				/*
				 * Inform the character of a note in progress and the possbility of continuation!
				 */
				if( ch->pcdata->in_progress )
					send_to_char( "You have a note in progress. Type NOTE WRITE to continue it.\r\n", ch );
			}
			return true;
		}
	}

	return false;
}



/*
 * Check if already playing.
 */

bool check_multi( DESCRIPTOR_DATA *d, const char *name )
{
	DESCRIPTOR_DATA *dold;

	return false;

	for( dold = first_descriptor; dold; dold = dold->next )
	{
		if( dold != d
			&& ( dold->character || dold->original )
			&& str_cmp( name, dold->original
				? dold->original->name : dold->character->name ) && !str_cmp( dold->host, d->host ) )
		{
			const char *ok = "194.234.177";
			const char *ok2 = "mudconnector.com";
			int iloop;

			if( get_trust( d->character ) >= LEVEL_STAFF
				|| get_trust( dold->original ? dold->original : dold->character ) >= LEVEL_STAFF )
				return false;
			for( iloop = 0; iloop < 11; iloop++ )
			{
				if( ok[iloop] != d->host[iloop] )
					break;
			}
			if( iloop >= 10 )
				return false;
			for( iloop = 0; iloop < 11; iloop++ )
			{
				if( ok2[iloop] != d->host[iloop] )
					break;
			}
			if( iloop >= 10 )
				return false;

			write_to_buffer( d, "Sorry multi-playing is not allowed ... have you other character quit first.\r\n", 0 );
			snprintf( log_buf, MAX_STRING_LENGTH, "%s attempting to multiplay with %s.",
				dold->original ? dold->original->name : dold->character->name, d->character->name );
			log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
			d->character->desc = NULL;
			free_char( d->character );
			d->character = NULL;
			return true;
		}
	}

	return false;

}

short check_playing( DESCRIPTOR_DATA *d, const char *name, bool kick )
{
	CHAR_DATA *ch;

	DESCRIPTOR_DATA *dold;
	int cstate;

	for( dold = first_descriptor; dold; dold = dold->next )
	{
		if( dold->original && dold->original->switched
			&& !IS_NPC( dold->original->switched )
			&& !str_cmp( dold->original->switched->name, name ) )
		{
			write_to_buffer( d, "Someone is currently switched into your character - try again.\r\n", 0 );
			return BERR;
		}
		if( dold != d
			&& ( dold->character || dold->original )
			&& !str_cmp( name, dold->original ? dold->original->name : dold->character->name ) )
		{
			cstate = dold->connected;
			ch = dold->original ? dold->original : dold->character;
			if( !ch->name || ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
			{
				write_to_buffer( d, "Already connected - try again.\r\n", 0 );
				snprintf( log_buf, MAX_STRING_LENGTH, "%s already connected.", ch->name );
				log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
				return BERR;
			}
			if( !kick )
				return true;
			write_to_buffer( d, "Already playing... Kicking off old connection.\r\n", 0 );
			write_to_buffer( dold, "Kicking off old connection... bye!\r\n", 0 );
			close_socket( dold, false );
			/*
			 * clear descriptor pointer to get rid of bug message in log
			 */
			d->character->desc = NULL;
			free_char( d->character );
			d->character = ch;
			ch->desc = d;
			ch->timer = 0;
			if( ch->switched )
				do_return( ch->switched, "" );
			ch->switched = NULL;
			send_to_char( "Reconnecting.\r\n", ch );
			act( AT_ACTION, "$n has reconnected, kicking off old link.", ch, NULL, NULL, TO_ROOM );
			snprintf( log_buf, MAX_STRING_LENGTH, "%s@%s reconnected, kicking off old link.", ch->name, d->host );
			log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->top_level ) );
			d->connected = cstate;
			return true;
		}
	}

	return false;
}



void stop_idling( CHAR_DATA *ch )
{
	if( !ch
		|| !ch->desc
		|| ch->desc->connected != CON_PLAYING || !ch->was_in_room || ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
		return;

	ch->timer = 0;
	char_from_room( ch );
	char_to_room( ch, ch->was_in_room );
	ch->was_in_room = NULL;
	act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
	return;
}

const char *obj_short( OBJ_DATA *obj )
{
	static char buf[MAX_STRING_LENGTH];

	if( obj->count > 1 )
	{
		snprintf( buf, MAX_STRING_LENGTH, "%s (%d)", obj->short_descr, obj->count );
		return buf;
	}
	return obj->short_descr;
}

/*
 * This function will remove all & and ^ color codes out of a string
 * Rewritten by Terell so that it won't cause static variable problems.
 */
const char *remand( const char *arg )
{
	static char ret[MAX_STRING_LENGTH];
	char *retptr;
	retptr = ret;

	if( arg == NULL )
		return NULL;

	for( ; *arg != '\0'; arg++ )
	{
		if( *arg == '&' && *( arg + 1 ) != '\0' )
			arg++;
		else if( *arg == '^' && *( arg + 1 ) != '\0' )
			arg++;
		else
		{
			*retptr = *arg;
			retptr++;
		}
	}
	*retptr = '\0';
	return ret;
}

/*
 * The primary output interface for formatted output.
 */
 /* Major overhaul. -- Alty */
#define NAME(ch)	(IS_NPC(ch) ? ch->short_descr : ch->name)
char *act_string( const char *format, CHAR_DATA *to, CHAR_DATA *ch, const void *arg1, const void *arg2 )
{
	static const char *const he_she[] = { "they", "he", "she" };
	static const char *const him_her[] = { "them", "him", "her" };
	static const char *const his_her[] = { "theirs", "his", "her" };
	static char buf[MAX_STRING_LENGTH];
	char fname[MAX_INPUT_LENGTH];
	char *point = buf;
	char temp[MAX_STRING_LENGTH];
	const char *str = format;
	const char *i;
	bool should_upper = false;
	CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
	OBJ_DATA *obj1 = ( OBJ_DATA * ) arg1;
	OBJ_DATA *obj2 = ( OBJ_DATA * ) arg2;

	if( str[0] == '$' )
		DONT_UPPER = false;

	while( *str != '\0' )
	{
		if( *str == '.' || *str == '?' || *str == '!' )
			should_upper = true;
		else if( should_upper == true && !isspace( *str ) && *str != '$' )
			should_upper = false;

		if( *str != '$' )
		{
			*point++ = *str++;
			continue;
		}
		++str;
		if( !arg2 && *str >= 'A' && *str <= 'Z' )
		{
			bug( "%s: missing arg2 for code %c: %s", __func__, *str, format );
			i = " <@@@> ";
		}
		else
		{
			switch( *str )
			{
			default:
				bug( "%s: bad code %c.", __func__, *str );
				i = " <@@@> ";
				break;

			case 'd':
				if( !arg2 || ( ( char * ) arg2 )[0] == '\0' )
					i = "door";
				else
				{
					one_argument( ( char * ) arg2, fname );
					i = fname;
				}
				break;

			case 'e':
				if( ch->sex > 2 || ch->sex < 0 )
				{
					bug( "%s: player %s has sex set at %d!", __func__, ch->name, ch->sex );
					i = should_upper ? "It" : "it";
				}
				else
					i = should_upper ?
					!can_see( to, ch ) ? "It" : capitalize( he_she[URANGE( 0, ch->sex, 2 )] ) :
					!can_see( to, ch ) ? "it" : he_she[URANGE( 0, ch->sex, 2 )];
				break;

			case 'E':
				if( vch->sex > 2 || vch->sex < 0 )
				{
					bug( "%s: player %s has sex set at %d!", __func__, vch->name, vch->sex );
					i = should_upper ? "It" : "it";
				}
				else
					i = should_upper ?
					!can_see( to, vch ) ? "It" : capitalize( he_she[URANGE( 0, vch->sex, 2 )] ) :
					!can_see( to, vch ) ? "it" : he_she[URANGE( 0, vch->sex, 2 )];
				break;

			case 'm':
				if( ch->sex > 2 || ch->sex < 0 )
				{
					bug( "%s: player %s has sex set at %d!", __func__, ch->name, ch->sex );
					i = should_upper ? "It" : "it";
				}
				else
					i = should_upper ?
					!can_see( to, ch ) ? "It" : capitalize( him_her[URANGE( 0, ch->sex, 2 )] ) :
					!can_see( to, ch ) ? "it" : him_her[URANGE( 0, ch->sex, 2 )];
				break;

			case 'M':
				if( vch->sex > 2 || vch->sex < 0 )
				{
					bug( "%s: player %s has sex set at %d!", __func__, vch->name, vch->sex );
					i = should_upper ? "It" : "it";
				}
				else
					i = should_upper ?
					!can_see( to, vch ) ? "It" : capitalize( him_her[URANGE( 0, vch->sex, 2 )] ) :
					!can_see( to, vch ) ? "it" : him_her[URANGE( 0, vch->sex, 2 )];
				break;

			case 'n':
				if( !can_see( to, ch ) )
					i = "Someone";
				else
				{
					snprintf( temp, sizeof( temp ), "%s", ( to ? PERS( ch, to ) : NAME( ch ) ) );
					i = temp;
				}
				break;

			case 'N':
				if( !can_see( to, vch ) )
					i = "Someone";
				else
				{
					snprintf( temp, sizeof( temp ), "%s", ( to ? PERS( vch, to ) : NAME( vch ) ) );
					i = temp;
				}
				break;

			case 'p':
				if( !obj1 )
				{
					bug( "%s: $p used with NULL obj1!", __func__ );
					i = "something";
				}
				else
					i = should_upper ? ( ( !to || can_see_obj( to, obj1 ) ) ? capitalize( obj_short( obj1 ) ) : "Something" )
					: ( ( !to || can_see_obj( to, obj1 ) ) ? obj_short( obj1 ) : "something" );
				break;

			case 'P':
				if( !obj2 )
				{
					bug( "%s: $P used with NULL obj2!", __func__ );
					i = "something";
				}
				else
					i = should_upper ? ( !to || can_see_obj( to, obj2 ) ? capitalize( obj_short( obj2 ) ) : "Something" )
					: ( !to || can_see_obj( to, obj2 ) ? obj_short( obj2 ) : "something" );
				break;

			case 'q':
				i = ( to == ch ) ? "" : "s";
				break;
			case 'Q':
				i = ( to == ch ) ? "your" : his_her[URANGE( 0, ch->sex, 2 )];
				break;

			case 's':
				if( ch->sex > 2 || ch->sex < 0 )
				{
					bug( "%s: player %s has sex set at %d!", __func__, ch->name, ch->sex );
					i = should_upper ? "It" : "it";
				}
				else
					i = should_upper ?
					!can_see( to, ch ) ? "It" : capitalize( his_her[URANGE( 0, ch->sex, 2 )] ) :
					!can_see( to, ch ) ? "it" : his_her[URANGE( 0, ch->sex, 2 )];
				break;

			case 'S':
				if( vch->sex > 2 || vch->sex < 0 )
				{
					bug( "%s: player %s has sex set at %d!", __func__, vch->name, vch->sex );
					i = should_upper ? "It" : "it";
				}
				else
					i = should_upper ?
					!can_see( to, vch ) ? "It" : capitalize( his_her[URANGE( 0, vch->sex, 2 )] ) :
					!can_see( to, vch ) ? "it" : his_her[URANGE( 0, vch->sex, 2 )];
				break;

			case 't':
				i = ( char * ) arg1;
				break;

			case 'T':
				i = ( char * ) arg2;
				break;
			}
		}
		++str;
		while( ( *point = *i ) != '\0' )
			++point, ++i;
	}
	mudstrlcpy( point, "\r\n", MAX_STRING_LENGTH );
	if( !DONT_UPPER )
	{
		bool bUppercase = true;     //Always uppercase first letter
		char *astr = buf;
		for( char c = *astr; c; c = *++astr )
		{
			if( c == '&' )
			{
				//Color Code
				c = *++astr;     //Read Color Code
				if( c == '[' )
				{
					//Extended color code, skip until ']'
					do { c = *++astr; } while( c && c != ']' );
				}

				if( !c )
					break;
			}
			else if( bUppercase && isalpha( c ) )
			{
				*astr = toupper( c );
				bUppercase = false;
			}
		}
	}
	return buf;
}

#undef NAME

void act( short AType, const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
	const char *txt;
	CHAR_DATA *to;
	CHAR_DATA *vch = ( CHAR_DATA * ) arg2;

	/*
	 * Discard null and zero-length messages.
	 */
	if( !format || format[0] == '\0' )
		return;

	if( !ch )
	{
		bug( "Act: null ch. (%s)", format );
		return;
	}

	if( !ch->in_room )
		to = NULL;
	else if( type == TO_CHAR )
		to = ch;
	else if( type == TO_MUD )
		to = first_char;
	else
		to = ch->in_room->first_person;


	/*
	 * ACT_SECRETIVE handling
	 */
	if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_SECRETIVE ) && type != TO_CHAR )
		return;

	if( type == TO_VICT )
	{
		if( !vch )
		{
			bug( "Act: null vch with TO_VICT." );
			bug( "%s (%s)", ch->name, format );
			return;
		}
		if( !vch->in_room )
		{
			bug( "Act: vch in NULL room!" );
			bug( "%s -> %s (%s)", ch->name, vch->name, format );
			return;
		}

		if( is_ignoring( ch, vch ) )
		{
			/*
			 * continue unless speaker is an immortal
			 */
			if( !IS_IMMORTAL( vch ) || ch->top_level > vch->top_level )
				return;
			else
			{
				set_char_color( AT_PLAIN, ch );
				ch_printf( ch, "You attempt to ignore %s, but are unable to do so.\r\n", vch->name );
			}
		}

		to = vch;
		/*	to = vch->in_room->first_person;*/
	}

	if( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
	{
		OBJ_DATA *to_obj;

		txt = act_string( format, NULL, ch, arg1, arg2 );
		if( HAS_PROG( to->in_room, ACT_PROG ) )
			rprog_act_trigger( txt, to->in_room, ch, ( OBJ_DATA * ) arg1, ( void * ) arg2 );
		for( to_obj = to->in_room->first_content; to_obj; to_obj = to_obj->next_content )
			if( HAS_PROG( to_obj->pIndexData, ACT_PROG ) )
				oprog_act_trigger( txt, to_obj, ch, ( OBJ_DATA * ) arg1, ( void * ) arg2 );
	}

	/*
	 * Anyone feel like telling me the point of looping through the whole
	 * room when we're only sending to one char anyways..? -- Alty
	 */
	for( ; to; to = ( type == TO_MUD ) ? to->next : ( type == TO_CHAR || type == TO_VICT ) ? NULL : to->next_in_room )
	{
		if( ( !to->desc && ( IS_NPC( to ) && !HAS_PROG( to->pIndexData, ACT_PROG ) ) ) || !IS_AWAKE( to ) )
			continue;

		if( type == TO_CHAR && to != ch )
			continue;
		if( type == TO_VICT && ( to != vch || to == ch ) )
			continue;
		if( type == TO_ROOM && to == ch )
			continue;
		if( type == TO_NOTVICT && ( to == ch || to == vch ) )
			continue;
		if( type == TO_MUD && ( to == ch || to == vch ) )
			continue;
		txt = act_string( format, to, ch, arg1, arg2 );
		if( to->desc )
		{
			set_char_color( AType, to );
			send_to_char( txt, to );
		}
		if( MOBtrigger )
		{
			/*
			 * Note: use original string, not string with ANSI. -- Alty
			 */
			mprog_act_trigger( txt, to, ch, ( OBJ_DATA * ) arg1, ( void * ) arg2 );
		}
	}
	MOBtrigger = true;
	return;
}

CMDF( do_gocial )
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	SOCIALTYPE *social;
	char command[MAX_INPUT_LENGTH];
	char *txt;

	argument = one_argument( argument, command );

	/*	if( !has_rate( ch, RATE_CHANNELS ) )
		{
			send_to_char( "You don't rate channels! HELP RATE for more info.\r\n", ch );
			return;
		}*/

	if( command[0] == '\0' )
	{
		send_to_char( "Gocial what?\r\n", ch );
		return;
	}

	REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
	if( xIS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if( !IS_NPC( ch ) && ( xIS_SET( ch->act, PLR_SILENCE ) ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	if( ( social = find_social( command ) ) == NULL )
	{
		send_to_char( "Pick a correct social to use.\r\n", ch );
		return;
	}

	if( IS_SET( ch->deaf, CHANNEL_GOCIAL ) && !IS_IMMORTAL( ch ) )
	{
		act( AT_PLAIN, "You have Gocial turned off... try chan +gocial first.", ch, NULL, NULL, TO_CHAR );
		return;
	}

	switch( ch->position )
	{
	case POS_SLEEPING:
		if( !str_cmp( social->name, "snore" ) )
			break;
		send_to_char( "In your dreams, or what?\r\n", ch );
		return;
	}

	one_argument( argument, arg );
	victim = NULL;

	if( arg[0] == '\0' )
	{
		snprintf( buf, MAX_STRING_LENGTH, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->others_no_arg );
		for( d = first_descriptor; d != NULL; d = d->next )
		{
			CHAR_DATA *vch;
			vch = d->original ? d->original : d->character;
			if( d->connected == CON_PLAYING && d->character != ch && !IS_SET( vch->deaf, CHANNEL_GOCIAL ) )
			{
				act( AT_SOCIAL, buf, ch, NULL, vch, TO_VICT );
			}
		}

		snprintf( buf, MAX_STRING_LENGTH, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->char_no_arg );
		act( AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR );
		return;
	}

	if( ( ( victim = get_char_world( ch, arg ) ) == NULL ) || ( !can_see( ch, victim ) ) )
	{
		send_to_char( "That person isn't logged on!\r\n", ch );
		return;
	}
	if( IS_NPC( victim ) )
	{
		send_to_char( "That person isn't logged on!\r\n", ch );
		return;
	}

	if( victim == ch )
	{
		snprintf( buf, MAX_STRING_LENGTH, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->others_auto );
		for( d = first_descriptor; d != NULL; d = d->next )
		{
			CHAR_DATA *vch;
			vch = d->original ? d->original : d->character;
			if( d->connected == CON_PLAYING && d->character != ch && !IS_SET( vch->deaf, CHANNEL_GOCIAL ) )
			{
				act( AT_SOCIAL, buf, ch, NULL, vch, TO_VICT );
			}
		}

		snprintf( buf, MAX_STRING_LENGTH, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->char_auto );
		act( AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR );
		return;
	}

	else
	{
		if( !social->others_found )
			txt = NULL;
		else
			txt = act_string( social->others_found, NULL, ch, NULL, victim );
		snprintf( buf, MAX_STRING_LENGTH, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", txt );
		for( d = first_descriptor; d != NULL; d = d->next )
		{
			CHAR_DATA *vch;
			vch = d->original ? d->original : d->character;
			if( d->connected == CON_PLAYING && d->character != ch &&
				d->character != victim && !IS_SET( vch->deaf, CHANNEL_GOCIAL ) )
			{
				if( txt != NULL )
					send_to_char( buf, vch );
				else
					send_to_char( "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z(null)\r\n", vch );
			}
		}

		snprintf( buf, MAX_STRING_LENGTH, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->char_found );
		act( AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR );
		snprintf( buf, MAX_STRING_LENGTH, "&G[&g|&BG&bo&Bc&bi&Ba&bl&g|&G> &z%s", social->vict_found );
		act( AT_SOCIAL, buf, ch, NULL, victim, TO_VICT );
		return;
	}
}

CMDF( do_name )
{
	char ucase_argument[MAX_STRING_LENGTH];
	char fname[1024];
	struct stat fst;
	CHAR_DATA *tmp;

	if( !NOT_AUTHED( ch ) || ch->pcdata->auth_state != 2 )
	{
		send_to_char( "Huh?\r\n", ch );
		return;
	}

	mudstrlcpy( ucase_argument, argument, MAX_STRING_LENGTH );
	ucase_argument[0] = UPPER( argument[0] );

	if( !check_parse_name( argument ) )
	{
		send_to_char( "Illegal name, try another.\r\n", ch );
		return;
	}

	if( !str_cmp( ch->name, argument ) )
	{
		send_to_char( "That's already your name!\r\n", ch );
		return;
	}

	for( tmp = first_char; tmp; tmp = tmp->next )
	{
		if( !str_cmp( argument, tmp->name ) )
			break;
	}

	if( tmp )
	{
		send_to_char( "That name is already taken.  Please choose another.\r\n", ch );
		return;
	}

	snprintf( fname, sizeof(fname), "%s%c/%s", PLAYER_DIR, tolower( argument[0] ), capitalize( argument ) );
	if( stat( fname, &fst ) != -1 )
	{
		send_to_char( "That name is already taken.  Please choose another.\r\n", ch );
		return;
	}

	STRFREE( ch->name );
	ch->name = STRALLOC( argument );
	STRFREE( ch->pcdata->title );
	ch->pcdata->title = STRALLOC( argument );
	send_to_char( "Your name has been changed.  Thank You for changing it.\r\n", ch );
	ch->pcdata->auth_state = 0;
	return;
}

char *default_prompt( CHAR_DATA *ch )
{
	static char buf[MAX_STRING_LENGTH];
	static char buf1[MAX_STRING_LENGTH];
	static char buf2[MAX_STRING_LENGTH];
	static SHIP_DATA *ship;
	strcpy( buf, "" );
	strcpy( buf2, "" );
	if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AFK ) )
	{
		strcat( buf, "&r---==&RYou're currently &w[&zA&WF&zK&w]&R: " );
		return buf;
	}

	if( ( ship = ship_from_cockpit( ch->in_room->vnum ) ) != NULL )
	{
		if( IS_SET( ship->flags, SUIT_INGRID ) )
		{
			snprintf( buf, MAX_STRING_LENGTH, "&B[&zArmor&B] &c%d&z/&C%d  &B[&zEnergy&B] &c%d&z/&C%d  &B[&zTarget&B] &c%s&w ", ship->frame,
				ship->framemax, ship->energy, ship->maxenergy, ship->target0 ? ship->target0->name : "None" );
			//       strcat( buf, buf2 );
			return buf;
		}
	}
	if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_FBALZHUR ) )
	{
		strcat( buf, "&B[&zHealth&B] &c500&z/&C500  &B[&zMovement&B] &c1000&z/&C1000" );
		strcat( buf, "  &B[&zMoney&B] &c0" );
		strcat( buf, "&C >&w " );
		return buf;
	}


	if( IS_SET( ch->pcdata->cybaflags, CYBA_HASGUN ) )
	{
		strcat( buf,
			"&B[&zEn&B] &c%x&C/&c%X   &B[&zCL&B] &c%C   &B[&zType&B]&c %w\r\n&B[&zHealth&B] &c%h&z/&C%H  &B[&zMovement&B] &c%v&z/&C%V" );
	}
	else
	{
		strcat( buf, "&B[&zHealth&B] &c%h&z/&C%H  &B[&zMovement&B] &c%v&z/&C%V" );
	}
	snprintf( buf1, MAX_STRING_LENGTH, "  &B[&zMoney&B] &c%s", num_punct( ch->gold ) );
	strcat( buf2, buf );
	strcat( buf, buf1 );
	strcat( buf, "&C >&w " );
	return buf;
}

int getcolor( char clr )
{
	static const char colors[17] = "xrgObpcwzRGYBPCW";
	int r;

	for( r = 0; r < 16; r++ )
		if( clr == colors[r] )
			return r;
	return -1;
}

void display_prompt( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch = d->character;
	CHAR_DATA *och = ( d->original ? d->original : d->character );
	bool ansi = ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) );
	const char *prompt;
	char buf[MAX_STRING_LENGTH];
	char *pbuf = buf;
	unsigned int pstat = 0;

	if( !ch )
	{
		bug( "display_prompt: NULL ch" );
		return;
	}

	if( !IS_NPC( ch ) && ch->substate != SUB_NONE && ch->pcdata->subprompt && ch->pcdata->subprompt[0] != '\0' )
		prompt = ch->pcdata->subprompt;
	else if( IS_NPC( ch ) || !ch->pcdata->prompt || !*ch->pcdata->prompt )
		prompt = default_prompt( ch );
	else
		prompt = ch->pcdata->prompt;

	if( ansi )
	{
		mudstrlcpy( pbuf, ANSI_RESET, MAX_STRING_LENGTH );
		d->prevcolor = 0x08;
		pbuf += 4;
	}
	/*
	 * Clear out old color stuff
	 */
	for( ; *prompt; prompt++ )
	{
		/*
		 * '%' = prompt commands
		 * Note: foreground changes will revert background to 0 (black)
		 */
		if( *prompt != '%' )
		{
			*( pbuf++ ) = *prompt;
			continue;
		}
		++prompt;
		if( !*prompt )
			break;
		if( *prompt == *( prompt - 1 ) )
		{
			*( pbuf++ ) = *prompt;
			continue;
		}
		switch( *( prompt - 1 ) )
		{
		default:
			bug( "Display_prompt: bad command char '%c'.", *( prompt - 1 ) );
			break;
		case '%':
			*pbuf = '\0';
			pstat = 0x80000000;
			switch( *prompt )
			{
			case '%':
				*pbuf++ = '%';
				*pbuf = '\0';
				break;
			case 'a':
				if( ch->top_level >= 10 )
					pstat = ch->alignment;
				else if( IS_GOOD( ch ) )
					mudstrlcpy( pbuf, "good", MAX_STRING_LENGTH );
				else if( IS_EVIL( ch ) )
					mudstrlcpy( pbuf, "evil", MAX_STRING_LENGTH );
				else
					mudstrlcpy( pbuf, "neutral", MAX_STRING_LENGTH );
				break;
			case 'C':
				pstat = ch->pcdata->chargelevel;
				break;
			case 'h':
				pstat = ch->hit;
				break;
			case 'H':
				pstat = ch->max_hit;
				break;
			case 'u':
				pstat = num_descriptors;
				break;
			case 'U':
				pstat = sysdata.maxplayers;
				break;
			case 'v':
				pstat = ch->move;
				break;
			case 'V':
				pstat = ch->max_move;
				break;
			case 'w':
				if( ch->pcdata->specialweapon == 0 )
					mudstrlcpy( pbuf, "Normal", MAX_STRING_LENGTH );
				else if( ch->pcdata->specialweapon == 1 )
					mudstrlcpy( pbuf, "Bubble", MAX_STRING_LENGTH );
				else if( ch->pcdata->specialweapon == 2 )
					mudstrlcpy( pbuf, "Smog", MAX_STRING_LENGTH );
				else if( ch->pcdata->specialweapon == 3 )
					mudstrlcpy( pbuf, "Tornado", MAX_STRING_LENGTH );
				else if( ch->pcdata->specialweapon == 4 )
					mudstrlcpy( pbuf, "Life Sap", MAX_STRING_LENGTH );
				else
					mudstrlcpy( pbuf, "Bug", MAX_STRING_LENGTH );
				break;
			case 'x':
				pstat = ch->pcdata->xenergy;
				break;
			case 'X':
				pstat = ch->pcdata->xenergymax;
				break;
			case 'g':
				pstat = ch->gold;
				break;
			case 'r':
				if( IS_IMMORTAL( och ) )
					pstat = ch->in_room->vnum;
				break;
			case 'R':
				if( xIS_SET( och->act, PLR_ROOMVNUM ) )
					snprintf( pbuf, MAX_STRING_LENGTH, "<#%d> ", ch->in_room->vnum );
				break;
			case 'i':
				if( ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_WIZINVIS ) ) ||
					( IS_NPC( ch ) && xIS_SET( ch->act, ACT_MOBINVIS ) ) )
					snprintf( pbuf, MAX_STRING_LENGTH, "(Invis %d) ", ( IS_NPC( ch ) ? ch->mobinvis : ch->pcdata->wizinvis ) );
				else if( IS_AFFECTED( ch, AFF_INVISIBLE ) )
					snprintf( pbuf, MAX_STRING_LENGTH, "(Invis) " );
				break;
			case 'I':
				pstat = ( IS_NPC( ch ) ? ( xIS_SET( ch->act, ACT_MOBINVIS ) ? ch->mobinvis : 0 )
					: ( xIS_SET( ch->act, PLR_WIZINVIS ) ? ch->pcdata->wizinvis : 0 ) );
				break;
			}
			if( pstat != 0x80000000 )
				snprintf( pbuf, MAX_STRING_LENGTH - strlen( buf ), "%d", pstat );
			pbuf += strlen( pbuf );
			break;
		}
	}
	*pbuf = '\0';
	send_to_char( buf, ch );
	return;
}

void set_pager_input( DESCRIPTOR_DATA *d, const char *argument )
{
	while( isspace( *argument ) )
		argument++;
	d->pagecmd = *argument;
	return;
}

bool pager_output( DESCRIPTOR_DATA *d )
{
	const char *last;
	CHAR_DATA *ch;
	int pclines;
	int lines;
	bool ret;

	if( !d || !d->pagepoint || d->pagecmd == -1 )
		return true;
	ch = d->original ? d->original : d->character;
	pclines = UMAX( ch->pcdata->pagerlen, 5 ) - 1;
	switch( LOWER( d->pagecmd ) )
	{
	default:
		lines = 0;
		break;
	case 'b':
		lines = -1 - ( pclines * 2 );
		break;
	case 'r':
		lines = -1 - pclines;
		break;
	case 'q':
		d->pagetop = 0;
		d->pagepoint = NULL;
		flush_buffer( d, true );
		DISPOSE( d->pagebuf );
		d->pagesize = MAX_STRING_LENGTH;
		return true;
	}
	while( lines < 0 && d->pagepoint >= d->pagebuf )
		if( *( --d->pagepoint ) == '\n' )
			++lines;
	if( *d->pagepoint == '\n' && *( ++d->pagepoint ) == '\r' )
		++d->pagepoint;
	if( d->pagepoint < d->pagebuf )
		d->pagepoint = d->pagebuf;
	for( lines = 0, last = d->pagepoint; lines < pclines; ++last )
		if( !*last )
			break;
		else if( *last == '\n' )
			++lines;
	if( *last == '\r' )
		++last;
	if( last != d->pagepoint )
	{
		if( !write_to_descriptor( d, d->pagepoint, ( last - d->pagepoint ) ) )
			return false;
		d->pagepoint = last;
	}
	while( isspace( *last ) )
		++last;
	if( !*last )
	{
		d->pagetop = 0;
		d->pagepoint = NULL;
		flush_buffer( d, true );
		DISPOSE( d->pagebuf );
		d->pagesize = MAX_STRING_LENGTH;
		return true;
	}
	d->pagecmd = -1;
	if( xIS_SET( ch->act, PLR_ANSI ) )
		if( write_to_descriptor( d, ANSI_LBLUE, 0 ) == false )
			return false;
	if( ( ret = write_to_descriptor( d, "(C)ontinue, (R)efresh, (B)ack, (Q)uit: [C] ", 0 ) ) == false )
		return false;
	if( xIS_SET( ch->act, PLR_ANSI ) )
	{
		char buf[32];

		snprintf( buf, 32, "%s", color_str( d->pagecolor, ch ) );
		ret = write_to_descriptor( d, buf, 0 );
	}
	return ret;
}

