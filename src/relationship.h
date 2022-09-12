#define ISTDATING(ch,vch)        (is_name(ch->name, vch->pcdata->tspouse))
#define ISDATING(ch,vch)        (is_name(ch->name, vch->pcdata->spouse))
#define stc send_to_char
#define ptc printf_to_char
