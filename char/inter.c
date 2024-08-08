//
// original code from athena
// SQL conversion by Jioh L. Jung
//

#include "char.h"
#include <string.h>
#include <stdlib.h>


#define WISLIST_TTL	(30*1000)	// Wisper list TTL (30sec)


MYSQL mysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;
int sql_fields, sql_cnt;
char tmp_sql[65535];

int db_server_port = 3306;
char db_server_ip[16] = "127.0.0.1";
char db_server_id[32] = "ragnarok";
char db_server_pw[32] = "ragnarok";
char db_server_logindb[32] = "ragnarok";


// sending packet list
int inter_send_packet_length[]={
	-1,-1, 27, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	-1, 7, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	35,-1, 11,15, 34,29, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0,
	10,-1, 15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1,
	 9, 9, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	11,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
};
// recv. packet list
int inter_recv_packet_length[]={
	-1,-1, 5, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 6,-1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	72, 6,52,14, 10,29, 6,-1, 34, 0, 0, 0,  0, 0,  0, 0,
	-1, 6,-1, 0, 55,19, 6,-1, 14,-1,-1,-1, 14,19,186,-1,
	 5, 9, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
};

struct WisList {
	struct WisList* next;
	int id, fd;
	int count;
	unsigned long tick;
	unsigned char src[24];
	unsigned char dst[24];
	unsigned char msg[512];
	unsigned int len;
};
struct WisList *wis_list=NULL;
short wis_id=0;

// Wis list add
int add_wislist(struct WisList* list)
{
	
	list->next=wis_list;
	list->id=wis_id++;
	list->tick=gettick();
	wis_list=list;

	if(wis_id>30000)
		wis_id=0;
	return list->id;
}
// Wis list search
struct WisList *search_wislist(int id)
{
	struct WisList* p;
	for(p=wis_list;p;p=p->next){
		if( p->id == id )
			return p;
	}
	return NULL;
}
// Wis list del.
int del_wislist(int id)
{
	struct WisList* p=wis_list, **prev=&wis_list;
//	printf("del_wislist:start\n");
	for( ; p; prev=&p->next, p=p->next ){
		if( p->id == id ){
			*prev=p->next;
			free(p);
//			printf("del_wislist:ok\n");
			return 1;
		}
	}
//	printf("del_wislist:not found\n");
	return 0;
}
// Wis list alive check
int check_ttl_wislist()
{
	unsigned long tick=gettick();
	struct WisList* p=wis_list, **prev=&wis_list;
	for( ; p; prev=&p->next, p=p->next ){
		if( DIFF_TICK(tick, p->tick)>WISLIST_TTL ){
			*prev=p->next;
			free(p);
			p=*prev;
		}
	}
	return 0;
}

//--------------------------------------------------------

/*==========================================
 * read config file
 *------------------------------------------
 */
int inter_config_read(const char *cfgName) {
	printf ("start reading interserver configuration: %s\n",cfgName);
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		printf("file not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line, 1020, fp)){
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		if(strcmpi(w1,"db_server_ip")==0){
			strcpy(db_server_ip, w2);
			printf ("set db_server_ip : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_port")==0){
			db_server_port=atoi(w2);
			printf ("set db_server_port : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_id")==0){
			strcpy(db_server_id, w2);
			printf ("set db_server_id : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_pw")==0){
			strcpy(db_server_pw, w2);
			printf ("set db_server_pw : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_logindb")==0){
			strcpy(db_server_logindb, w2);
			printf ("set db_server_logindb : %s\n",w2);
		}
	}
	fclose(fp);
	
	printf ("success reading interserver configuration\n");

	return 0;
}

// save
/*
int inter_save() {
	// guild is save to file
	inter_guild_save();

	return 0;
}

int inter_save_timer(int tid, unsigned int tick, int id, int data){
	//printf ("interserver save count-tic...\n");
	inter_save();
	return 0;
}
*/

// initialize
int inter_init(const char *file)
{
	//int i;
	
	printf ("interserver initialize...\n");
	inter_config_read(file);
	
	//DB connection initialized
	mysql_init(&mysql_handle);
	printf("Connect DB server.... (inter server)\n");
	if(!mysql_real_connect(&mysql_handle, db_server_ip, db_server_id, db_server_pw,
		db_server_logindb ,db_server_port, (char *)NULL, 0)) {
			//pointer check
			printf("%s\n",mysql_error(&mysql_handle));
			exit(1);
	}
	else {
		printf ("connect success! (inter server)\n");
	}


	inter_storage_init();
	inter_party_sql_init();
	inter_guild_sql_init();
	inter_pet_sql_init();

	//printf ("interserver timer initializing : %d sec...\n",autosave_interval);
	//i=add_timer_interval(gettick()+autosave_interval,inter_save_timer,0,0,autosave_interval);

	return 0;
}

//--------------------------------------------------------

// GM message sending
int mapif_GMmessage(unsigned char *mes, int len)
{
	unsigned char buf[len];
	WBUFW(buf, 0) =0x3800;
	WBUFW(buf, 2) =len;
	memcpy(WBUFP(buf, 4), mes, len-4);
	mapif_sendall(buf, len);
	printf("inter server: GM[len:%d] - '%s'\n", len, mes);
	return 0;
}

// Wis sending
int mapif_wis_message(struct WisList *wl)
{
	unsigned char buf[1024];
	
	WBUFW(buf, 0) =0x3801;
	WBUFW(buf, 2)=6 + 48 +wl->len;
	WBUFW(buf, 4) =wl->id;
	memcpy(WBUFP(buf, 6), wl->src, 24);
	memcpy(WBUFP(buf, 30), wl->dst, 24);
	memcpy(WBUFP(buf, 54), wl->msg, wl->len);
	wl->count = mapif_sendall(buf, WBUFW(buf, 2));
//	printf("inter server wis: %d %d %ld\n", wl->id, wl->count, wl->tick);
	return 0;
}
// Wis sending result
int mapif_wis_end(struct WisList *wl, int flag)
{
	unsigned char buf[32];
	
	WBUFW(buf, 0) =0x3802;
	memcpy(WBUFP(buf, 2), wl->src, 24);
	WBUFB(buf, 26) =flag;
	mapif_send(wl->fd, buf, 27);
//	printf("inter server wis_end %d\n", flag);
	return 0;
}

//--------------------------------------------------------

// GM message sending
int mapif_parse_GMmessage(int fd)
{
	mapif_GMmessage(RFIFOP(fd, 4), RFIFOW(fd, 2));
	return 0;
}


// Wis sending request
int mapif_parse_WisRequest(int fd)
{
	struct WisList* wl = (struct WisList *) malloc(sizeof(struct WisList));
	if(wl==NULL){
		// Wis sending fail
		RFIFOSKIP(fd, RFIFOW(fd, 2));
		return 0;
	}
	check_ttl_wislist();
	
	wl->fd=fd;	// WisList set
	memcpy(wl->src, RFIFOP(fd, 4), 24);
	memcpy(wl->dst, RFIFOP(fd, 28), 24);
	wl->len=RFIFOW(fd, 2)-52;
	memcpy(wl->msg, RFIFOP(fd, 52), wl->len);
	
	add_wislist(wl);
	
	mapif_wis_message(wl);
	return 0;
}

// Wis sending result
int mapif_parse_WisReply(int fd)
{
	int id=RFIFOW(fd, 2), flag=RFIFOB(fd, 4);
	
	struct WisList* wl=search_wislist(id);
	
	if(wl==NULL){
		RFIFOSKIP(fd, 5);
		return 0;	// no such ID
	}
	
	if((--wl->count)==0 || flag!=1){
		mapif_wis_end(wl, flag);
		del_wislist(wl->id);
	}
	
	return 0;
}

//--------------------------------------------------------
int inter_parse_frommap(int fd)
{
	int cmd=RFIFOW(fd, 0);
	int len=0;

	// inter packet?
	if(cmd<0x3000 || cmd>=0x3000+( sizeof(inter_recv_packet_length)/
		sizeof(inter_recv_packet_length[0]) ) )
		return 0;

	// packet check
	if(	(len=inter_check_length(fd, inter_recv_packet_length[cmd-0x3000]))==0 )
		return 2;
	
	switch(cmd){
	case 0x3000: mapif_parse_GMmessage(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	default:
		if( inter_party_parse_frommap(fd) )
			break;
		if( inter_guild_parse_frommap(fd) )
			break;
		if( inter_storage_parse_frommap(fd) )
			break;
		if( inter_pet_parse_frommap(fd) )
			break;
		return 0;
	}
	RFIFOSKIP(fd, len );
	return 1;
}

// RFIFO check
int inter_check_length(int fd, int length)
{
	if(length==-1){	// v-len packet
		if(RFIFOREST(fd)<4)	// packet not yet
			return 0;
		length = RFIFOW(fd, 2);
	}
	
	if(RFIFOREST(fd)<length)	// packet not yet
		return 0;
	
	return length;
}

