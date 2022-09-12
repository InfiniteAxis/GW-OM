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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef __cplusplus
typedef unsigned char bool;
#endif
extern bool mud_down;

#define STR_HASH_SIZE	1024

struct hashstr_data
{
    struct hashstr_data *next; /* next hash element */
    unsigned short int links;  /* number of links to this string */
    unsigned short int length; /* length of string */
};

#ifndef __cplusplus
typedef unsigned char bool;
#endif

const char *str_alloc( const char *str );
const char *quick_link( const char *str );
int str_free( const char *str );
void show_hash( int count );
char *hash_stats( void );

struct hashstr_data *string_hash[STR_HASH_SIZE];

/*
 * Check hash table for existing occurance of string.
 * If found, increase link count, and return pointer,
 * otherwise add new string to hash table, and return pointer.
 */
const char *str_alloc( const char *str )
{
    int len, hash, psize;
    struct hashstr_data *ptr;

    len = strlen( str );
    psize = sizeof( struct hashstr_data );
    hash = len % STR_HASH_SIZE;
    for( ptr = string_hash[hash]; ptr; ptr = ptr->next )
        if( len == ptr->length && !strcmp( str, ( char * ) ptr + psize ) )
        {
            if( ptr->links < 65535 )
                ++ptr->links;
            return ( char * ) ptr + psize;
        }
    ptr = ( struct hashstr_data * ) malloc( len + psize + 1 );
    ptr->links = 1;
    ptr->length = len;
    if( len )
        strcpy( ( char * ) ptr + psize, str );
    /*     memcpy( (char *) ptr+psize, str, len+1 ); */
    else
        strcpy( ( char * ) ptr + psize, "" );
    ptr->next = string_hash[hash];
    string_hash[hash] = ptr;
    return ( char * ) ptr + psize;
}

/*
 * Used to make a quick copy of a string pointer that is known to be already
 * in the hash table.  Function increments the link count and returns the
 * same pointer passed.
 */
const char *quick_link( const char *str )
{
    struct hashstr_data *ptr;

    ptr = ( struct hashstr_data * ) ( str - sizeof( struct hashstr_data ) );
    if( ptr->links == 0 )
    {
        fprintf( stderr, "quick_link: bad pointer\n" );
        return NULL;
    }
    if( ptr->links < 65535 )
        ++ptr->links;
    return str;
}

/*
 * Used to remove a link to a string in the hash table.
 * If all existing links are removed, the string is removed from the
 * hash table and disposed of.
 * returns how many links are left, or -1 if an error occurred.
 */
int str_free( const char *str )
{
    int len, hash;
    struct hashstr_data *ptr, *ptr2, *ptr2_next;

    len = strlen( str );
    hash = len % STR_HASH_SIZE;
    ptr = ( struct hashstr_data * ) ( str - sizeof( struct hashstr_data ) );
    if( ptr->links == 65535 )  /* permanent */
        return ptr->links;
    if( ptr->links == 0 )
    {
        fprintf( stderr, "str_free: bad pointer\n" );
        return -1;
    }
    if( --ptr->links == 0 )
    {
        if( string_hash[hash] == ptr )
        {
            string_hash[hash] = ptr->next;
            free( ptr );
            return 0;
        }
        for( ptr2 = string_hash[hash]; ptr2; ptr2 = ptr2_next )
        {
            ptr2_next = ptr2->next;
            if( ptr2_next == ptr )
            {
                ptr2->next = ptr->next;
                free( ptr );
                return 0;
            }
        }
        fprintf( stderr, "str_free: pointer not found for string: %s\n", str );
        return -1;
    }
    return ptr->links;
}

void show_hash( int count )
{
    struct hashstr_data *ptr;
    int x, c;

    for( x = 0; x < count; x++ )
    {
        for( c = 0, ptr = string_hash[x]; ptr; ptr = ptr->next, c++ );
        fprintf( stderr, " %d", c );
    }
    fprintf( stderr, "\n" );
}

void hash_dump( int hash )
{
    struct hashstr_data *ptr;
    char *str;
    int c, psize;

    if( hash > STR_HASH_SIZE || hash < 0 )
    {
        fprintf( stderr, "hash_dump: invalid hash size\r\n" );
        return;
    }
    psize = sizeof( struct hashstr_data );
    for( c = 0, ptr = string_hash[hash]; ptr; ptr = ptr->next, c++ )
    {
        str = ( char * ) ( ( ( long ) ptr ) + psize );
        fprintf( stderr, "Len:%4d Lnks:%5d Str: %s\r\n", ptr->length, ptr->links, str );
    }
    fprintf( stderr, "Total strings in hash %d: %d\r\n", hash, c );
}

char *check_hash( const char *str )
{
    static char buf[1024];
    int len, hash, psize, p = 0, c;
    struct hashstr_data *ptr, *fnd;

    buf[0] = '\0';
    len = strlen( str );
    psize = sizeof( struct hashstr_data );
    hash = len % STR_HASH_SIZE;
    for( fnd = NULL, ptr = string_hash[hash], c = 0; ptr; ptr = ptr->next, c++ )
        if( len == ptr->length && !strcmp( str, ( char * ) ptr + psize ) )
        {
            fnd = ptr;
            p = c + 1;
        }
    if( fnd )
        snprintf( buf, 1024, "Hash info on string: %s\r\nLinks: %d  Position: %d/%d  Hash: %d  Length: %d\r\n",
            str, fnd->links, p, c, hash, fnd->length );
    else
        snprintf( buf, 1024, "%s not found.\r\n", str );
    return buf;
}

char *hash_stats( void )
{
    static char buf[1024];
    struct hashstr_data *ptr;
    int x, c, total, totlinks, unique, bytesused, wouldhave, hilink;

    totlinks = unique = total = bytesused = wouldhave = hilink = 0;
    for( x = 0; x < STR_HASH_SIZE; x++ )
    {
        for( c = 0, ptr = string_hash[x]; ptr; ptr = ptr->next, c++ )
        {
            total++;
            if( ptr->links == 1 )
                unique++;
            if( ptr->links > hilink )
                hilink = ptr->links;
            totlinks += ptr->links;
            bytesused += ( ptr->length + 1 + sizeof( struct hashstr_data ) );
            wouldhave += ( ( ptr->links * sizeof( struct hashstr_data ) ) + ( ptr->links * ( ptr->length + 1 ) ) );
        }
    }
    snprintf( buf, 1024,
        "Hash strings allocated:%8d  Total links  : %d\r\nString bytes allocated:%8d  Bytes saved  : %d\r\nUnique (wasted) links :%8d  Hi-Link count: %d\r\n",
        total, totlinks, bytesused, wouldhave - bytesused, unique, hilink );
    return buf;
}

void show_high_hash( int top )
{
    struct hashstr_data *ptr;
    int x, psize;
    char *str;

    psize = sizeof( struct hashstr_data );
    for( x = 0; x < STR_HASH_SIZE; x++ )
        for( ptr = string_hash[x]; ptr; ptr = ptr->next )
            if( ptr->links >= top )
            {
                str = ( char * ) ( ( ( long ) ptr ) + psize );
                fprintf( stderr, "Links: %5d  String: >%s<\r\n", ptr->links, str );
            }
}

bool in_hash_table( const char *str )
{
    int len, hash, psize;
    struct hashstr_data *ptr;

    len = strlen( str );
    psize = sizeof( struct hashstr_data );
    hash = len % STR_HASH_SIZE;
    for( ptr = string_hash[hash]; ptr; ptr = ptr->next )
        if( len == ptr->length && str == ( ( char * ) ptr + psize ) )
            return true;
    return false;
}

