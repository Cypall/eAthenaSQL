// $Id: login.c,v 1.2 2004/02/17 04:48:07 rovert Exp $
// original : login2.c 2003/01/28 02:29:17 Rev.1.1.1.1

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>

//add include for DBMS(mysql)
#include <mysql.h>

#include "strlib.h"
#include "timer.h"
/*
#include "core.h"
#include "socket.h"
#include "login.h"
#include "mmo.h"
#include "version.h"
#include "db.h"
*/

#include "../common/core.h"
#include "../common/socket.h"
#include "login.h"
#include "../common/mmo.h"
#include "../common/version.h"
#include "../common/db.h"

#ifdef PASSWORDENC
#include "md5calc.h"
#endif

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define J_MAX_MALLOC_SIZE 65535

//-----------------------------------------------------
// global variable
//-----------------------------------------------------
int account_id_count = START_ACCOUNT_NUM;
int server_num;
int new_account_flag = 0;
int login_port = 6900;

struct mmo_char_server server[MAX_SERVERS];
int server_fd[MAX_SERVERS];

MYSQL mysql_handle;

int ipban = 1;
int dynamic_account_ban = 1;
int dynamic_account_ban_class = 0;
int dynamic_pass_failure_ban = 1;
int dynamic_pass_failure_ban_time = 5;
int dynamic_pass_failure_ban_how_many = 3;
int dynamic_pass_failure_ban_how_long = 60;

int db_server_port = 3306;
char db_server_ip[16] = "127.0.0.1";
char db_server_id[32] = "ragnarok";
char db_server_pw[32] = "ragnarok";
char db_server_logindb[32] = "ragnarok";

char tmpsql[65535];

//-----------------------------------------------------

#define AUTH_FIFO_SIZE 256
struct {
  int account_id,login_id1,login_id2;
  int sex,delflag;
} auth_fifo[AUTH_FIFO_SIZE];

int auth_fifo_pos=0;


//-----------------------------------------------------

char admin_pass[64]="";
char gm_pass[64]="";
const int gm_start=704554,gm_last=704583;

static char md5key[20],md5keylen=16;

static struct dbt *gm_account_db;
char GM_account_filename[1024] = "conf/GM_account.txt";

//-----------------------------------------------------
//check user level
//-----------------------------------------------------
int isGM(int account_id)
{
	struct gm_account *p;
	p = numdb_search(gm_account_db, account_id);
	if( p == NULL) {
		return 0;
		//not GM
	}
	//return level
	return p->level;
}

//-----------------------------------------------------
// GM account initialize
//-----------------------------------------------------
int read_gm_account() {
	char line[8192];
	struct gm_account *p;
	FILE *fp;
	int c=0;

	gm_account_db = numdb_init();
	
	printf("gm_account reading start.....\n");
	
	if( (fp=fopen(GM_account_filename,"r")) ==NULL )
		return 1;
	while(fgets(line, sizeof(line), fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		p=malloc(sizeof(struct gm_account));
		if(p==NULL){
			printf("gm_account: out of memory!\n");
			exit(0);
		}
		if(sscanf(line,"%d %d", &p->account_id, &p->level) != 2 || p->level <= 0) {
			printf("gm_account: broken data [%s] line %d\n", GM_account_filename, c);
		}
		else {
			if(p->level > 99)
				p->level = 99;
			numdb_insert(gm_account_db, p->account_id, p);
		}
		c++;
	}
	fclose(fp);
	
	printf("gm_account: read done (%d gm account ID) \n",  c);
	
	return 0;
}
//-----------------------------------------------------
// Read Account database - mysql db
//-----------------------------------------------------
int mmo_auth_sqldb_init(void)
{
	printf("Login server init....\n");
	
	//memory initialize
	printf("memory initialize....\n");
	
	mysql_init(&mysql_handle);
	
	
	//DB connection start
	printf("Connect DB server....\n");
	if(!mysql_real_connect(&mysql_handle, db_server_ip, db_server_id, db_server_pw,
		db_server_logindb ,db_server_port, (char *)NULL, 0)) {
			//pointer check
			printf("%s\n",mysql_error(&mysql_handle));
			exit(1);
	}
	else {
		printf ("connect success!\n");
	}
	
	sprintf(tmpsql,"INSERT INTO `loginlog`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver', '100','login server started')");
	
	//query
	if(mysql_query(&mysql_handle, tmpsql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	
	return 0;
}
//-----------------------------------------------------
// DB server connect check
//-----------------------------------------------------
void mmo_auth_sqldb_sync(void) {
	//db connect check? or close?
	//ping pong DB server -if losted? then connect try. else crash.
}
//-----------------------------------------------------
// close DB
//-----------------------------------------------------
void mmo_db_close(void) {

	//set log.
	sprintf(tmpsql,"INSERT INTO `loginlog`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver','100', 'login server shutdown')");
	
	//query
	if(mysql_query(&mysql_handle, tmpsql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	
	//delete all server status
	sprintf(tmpsql,"DELETE FROM `sstatus`");
	//query
	if(mysql_query(&mysql_handle, tmpsql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	
	mysql_close(&mysql_handle);
	printf("close DB connect....\n");
}
//-----------------------------------------------------
// Make new account
//-----------------------------------------------------
int mmo_auth_sqldb_new( struct mmo_account* account,const char *tmpstr,char sex )
{
	//no need on DB version

	printf("Request new account.... - not support on this version\n");
	return 0;
}
//-----------------------------------------------------
// Make new account
//-----------------------------------------------------
int mmo_auth_new( struct mmo_account* account,const char *tmpstr,char sex )
{

	return 0;
}


//-----------------------------------------------------
// Auth
//-----------------------------------------------------
int mmo_auth( struct mmo_account* account ){
	
	printf ("auth start...\n");
	struct timeval tv;
	char tmpstr[256];
	char t_uid[256], t_pass[256];
	
	MYSQL_RES* 	sql_res ;
	MYSQL_ROW	sql_row ;
	//int 	sql_fields, sql_cnt;
	char md5str[md5keylen+32],md5bin[32];

	//auth start : time seed
	gettimeofday(&tv,NULL);
	strftime(tmpstr,24,"%Y-%m-%d %H:%M:%S",localtime(&(tv.tv_sec)));
	sprintf(tmpstr+19,".%03d",(int)tv.tv_usec/1000);
	
	jstrescapecpy (t_uid,account->userid);
	jstrescapecpy (t_pass, account->passwd);
	//make query
	sprintf(tmpsql, "SELECT `account_id`,`userid`,`user_pass`,`lastlogin`,`logincount`,`sex`,`level` FROM `login` WHERE `userid`='%s'"
		,t_uid);
	//login {account_id/userid/user_pass/lastlogin/logincount/sex/level}
	
	//query
	if(mysql_query(&mysql_handle, tmpsql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	sql_row = mysql_fetch_row(sql_res);	//row fetching
	if (!sql_row)  {
		//there's no id.
		printf ("auth failed no account %s %s %s\n", tmpstr, account->userid, account->passwd);
		mysql_free_result(sql_res);
		return 0;
	}
	else {
		if (atoi(sql_row[6]) == -1) {
			//id is banned
			mysql_free_result(sql_res);
			return 4;
		}
		else if (atoi(sql_row[6]) == -2) { //dynamic ban
			//id is banned
			mysql_free_result(sql_res);
			//add IP list.
			return -2;
		}
		printf ("account id ok encval:%d\n",account->passwdenc);
		int encpasswdok=0;
#ifdef PASSWORDENC
		if(account->passwdenc>0){
			printf ("start md5calc..\n");
			int j=account->passwdenc;
			if(j>2)j=1;
			do{
				if(j==1){
					sprintf(md5str,"%s%s",md5key,sql_row[2]);
				}else if(j==2){
					sprintf(md5str,"%s%s",sql_row[2],md5key);
				}else
					md5str[0]=0;
				printf ("j:%d mdstr:%s\n",j,md5str);
				MD5_String2binary(md5str,md5bin);
				encpasswdok = ( memcmp( account->passwd, md5bin, 16) == 0);
			}while(j<2 && !encpasswdok && (j++)!=account->passwdenc);
			//printf("key[%s] md5 [%s] ",md5key,md5);
			printf("client [%s] accountpass [%s]\n",account->passwd,sql_row[2]);
			printf ("end md5calc..\n");
		}
#endif
		if( (strcmp(account->passwd,sql_row[2]) && !encpasswdok)){
		  	if(account->passwdenc==0){
				printf ("auth failed pass error %s %s %s" RETCODE,tmpstr,account->userid,account->passwd);
			}
#ifdef PASSWORDENC
			else{
				char logbuf[1024],*p=logbuf;
				int j;
				p+=sprintf(p,"auth failed pass error %s %s recv-md5[",tmpstr,account->userid);
				for(j=0;j<16;j++)
					p+=sprintf(p,"%02x",((unsigned char *)account->passwd)[j]);
				p+=sprintf(p,"] calc-md5[");
				for(j=0;j<16;j++)
					p+=sprintf(p,"%02x",((unsigned char *)md5bin)[j]);
				p+=sprintf(p,"] md5key[");			
				for(j=0;j<md5keylen;j++)
					p+=sprintf(p,"%02x",((unsigned char *)md5key)[j]);
				p+=sprintf(p,"]" RETCODE);
				printf ("%s\n",p);
			}
#endif
			return 1;
		}
		printf ("auth ok %s %s" RETCODE, tmpstr,account->userid);
	}
	//login {account_id/userid/user_pass/lastlogin/logincount/sex/level}
	
	account->account_id = atoi(sql_row[0]);
	account->login_id1 = rand();
	account->login_id2 = rand();
	memcpy(tmpstr,sql_row[3],19);
	memcpy(account->lastlogin,tmpstr,24);
	account->sex = sql_row[5][0] == 'S' ? 2 : sql_row[5][0]=='M';

	sprintf(tmpsql, "UPDATE `login` SET `lastlogin` = NOW(), `logincount`=`logincount` +1  WHERE  `userid` = '%s'", sql_row[1]);
	mysql_free_result(sql_res) ; //resource free
	if(mysql_query(&mysql_handle, tmpsql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	return 100;
}

//-----------------------------------------------------
// char-server packet parse
//-----------------------------------------------------
int parse_fromchar(int fd){
	int i, id;

	for(id=0;id<MAX_SERVERS;id++)
		if(server_fd[id]==fd)
			break;
	
	if(id==MAX_SERVERS)
		session[fd]->eof=1;
	
	if(session[fd]->eof){
		for(i=0;i<MAX_SERVERS;i++)
			if(server_fd[i]==fd)
				server_fd[i]=-1;
		close(fd);
		//server delete
		sprintf(tmpsql,"DELETE FROM `sstatus` WHERE `index`='%d'",id);
		//query
		if(mysql_query(&mysql_handle, tmpsql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		}

		printf("Kill session - %d\n",fd);
		delete_session(fd);
		return 0;
	}
	while(RFIFOREST(fd)>=2){
		if (RFIFOW(fd,0)!=0x2714)
			printf("char_parse: %d %d packet case=%d\n",fd,RFIFOREST(fd),RFIFOW(fd, 0));
		switch(RFIFOW(fd,0)){
			case 0x2712:
				printf("auth?\n");
				if(RFIFOREST(fd)<15)
					return 0;
				for(i=0;i<AUTH_FIFO_SIZE;i++){
					if(auth_fifo[i].account_id==RFIFOL(fd,2) &&
						auth_fifo[i].login_id1==RFIFOL(fd,6) &&
						/*auth_fifo[i].login_id2==RFIFOL(fd,10) &&*/
						auth_fifo[i].sex==RFIFOB(fd,14) &&
						!auth_fifo[i].delflag){
							auth_fifo[i].delflag=1;
							printf("auth -> %d\n",i);
							break;
					}
				}
				WFIFOW(fd,0)=0x2713;
				WFIFOL(fd,2)=RFIFOL(fd,2);
				if(i!=AUTH_FIFO_SIZE){
					WFIFOB(fd,6)=0;
				}
				else {
					WFIFOB(fd,6)=1;
				}
				WFIFOSET(fd,7);
				RFIFOSKIP(fd,15);
				break;
			case 0x2714:
				//how many users on world? (update)
				if (server[id].users!=RFIFOL(fd,2))
					printf("set users %s : %d\n",server[id].name,RFIFOL(fd,2));
				server[id].users=RFIFOL(fd,2);
				
				sprintf(tmpsql,"UPDATE `sstatus` SET `user` = '%d' WHERE `index` = '%d'", server[id].users, id);
					//query
					if(mysql_query(&mysql_handle, tmpsql) ) {
							printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
				}
				RFIFOSKIP(fd,6);
				break;
			case 0x2720:	// GM
				if(RFIFOREST(fd)<4)
					return 0;
				if(RFIFOREST(fd)<RFIFOW(fd,2))
					return 0;
				//oldacc=RFIFOL(fd,4);
				printf("change GM isn't support in this login server version.\n");
				printf("change GM error 0 %s\n", RFIFOP(fd, 8));

				RFIFOSKIP(fd, RFIFOW(fd, 2));
				WFIFOW(fd, 0) =0x2721;
				WFIFOL(fd, 2) =RFIFOL(fd,4);//oldacc;
				WFIFOL(fd, 6) =0;//newacc;
				WFIFOSET(fd, 10);
				return 0;
    default:
      printf ("close session connection...\n");
      close(fd);
      session[fd]->eof=1;
      return 0;
    }
  }
  return 0;
}

//-----------------------------------------------------
// packet parsing
//-----------------------------------------------------
int parse_login(int fd){
	//int len;
	
	MYSQL_RES* 	sql_res ;
	MYSQL_ROW	sql_row ;
	
	char t_uid[100];
	//int 	sql_fields, sql_cnt;
	struct mmo_account account;
	
	int result, i;
	unsigned char *p=(unsigned char *) &session[fd]->client_addr.sin_addr;
	
	if(session[fd]->eof){
		for(i=0;i<MAX_SERVERS;i++)
			if(server_fd[i]==fd)
				server_fd[i]=-1;
		close(fd);
		delete_session(fd);
		return 0;
	}
	
	if (ipban > 0) {
		//ip ban
		//p[0], p[1], p[2], p[3]
		//request DB connection
		//check
		sprintf (tmpsql, "SELECT count(*) FROM `ipbanlist` WHERE `list` = '%d.*.*.*' OR `list` = '%d.%d.*.*' OR `list` = '%d.%d.%d.*' OR `list` = '%d.%d.%d.%d'", 
		  p[0], p[0], p[1], p[0], p[1], p[2], p[0], p[1], p[2], p[3]);
		if(mysql_query(&mysql_handle, tmpsql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		}
		
		sql_res = mysql_store_result(&mysql_handle) ;
		sql_row = mysql_fetch_row(sql_res);	//row fetching
			
		if (atoi(sql_row[0]) >0)  {
			//ip ban ok.
			printf ("packet from banned ip : %d.%d.%d.%d" RETCODE, p[0], p[1], p[2], p[3]);
			sprintf(tmpsql,"INSERT INTO `loginlog`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', 'unknown','-3', 'ip banned')", p[0], p[1], p[2], p[3]);
	
			//query
			if(mysql_query(&mysql_handle, tmpsql) ) {
					printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
			}
			printf ("close session connection...\n");
			
			//close connection
			close(fd);
			session[fd]->eof=1;
			
			mysql_free_result(sql_res);
			return 0;
		}
		else {
			printf ("packet from ip (ban check ok) : %d.%d.%d.%d" RETCODE, p[0], p[1], p[2], p[3]);
		}
		mysql_free_result(sql_res);
	}

	if(RFIFOW(fd,0)<30000)
		printf("parse_login : %d %d packet case=%d\n",fd,RFIFOREST(fd),RFIFOW(fd,0));
	while(RFIFOREST(fd)>=2){
		switch(RFIFOW(fd,0)){
			case 0x64:		// request client login
			case 0x01dd:	// request client login with encrypt
				if(RFIFOREST(fd)< ((RFIFOW(fd, 0) ==0x64)?55:47))
					return 0;
			
				printf("client connection request %s from %d.%d.%d.%d\n", RFIFOP(fd, 6), p[0], p[1], p[2], p[3]);
				
				account.userid = RFIFOP(fd, 6);
				account.passwd = RFIFOP(fd, 30);
#ifdef PASSWORDENC
		account.passwdenc= (RFIFOW(fd,0)==0x64)?0:PASSWORDENC;
#else
		account.passwdenc=0;
#endif
		result=mmo_auth(&account);

		jstrescapecpy(t_uid,RFIFOP(fd, 6));
		if(result==100){
			if (p[0] != 127) {
				sprintf(tmpsql,"INSERT INTO `loginlog`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s','100', 'login ok')", p[0], p[1], p[2], p[3], t_uid);
		
				//query
				if(mysql_query(&mysql_handle, tmpsql) ) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
				}
			}
			
			server_num=0;
			for(i=0;i<MAX_SERVERS;i++){
				if(server_fd[i]>=0){
					WFIFOL(fd,47+server_num*32) = server[i].ip;
					WFIFOW(fd,47+server_num*32+4) = server[i].port;
					memcpy(WFIFOP(fd,47+server_num*32+6), server[i].name, 20);
					WFIFOW(fd,47+server_num*32+26) = server[i].users;
					WFIFOW(fd,47+server_num*32+28) = server[i].maintenance;
					WFIFOW(fd,47+server_num*32+30) = server[i].new;
					server_num++;
				}
			}
			WFIFOW(fd,0)=0x69;
			WFIFOW(fd,2)=47+32*server_num;
			WFIFOL(fd,4)=account.login_id1;
			WFIFOL(fd,8)=account.account_id;
			WFIFOL(fd,12)=account.login_id2;
			WFIFOL(fd,16)=0;
			memcpy(WFIFOP(fd,20),account.lastlogin,24);
			WFIFOB(fd,46)=account.sex;
			WFIFOSET(fd,47+32*server_num);
			if(auth_fifo_pos>=AUTH_FIFO_SIZE){
				auth_fifo_pos=0;
			}
			auth_fifo[auth_fifo_pos].account_id=account.account_id;
			auth_fifo[auth_fifo_pos].login_id1=account.login_id1;
			auth_fifo[auth_fifo_pos].login_id2=account.login_id2;
			auth_fifo[auth_fifo_pos].sex=account.sex;
			auth_fifo[auth_fifo_pos].delflag=0;
			auth_fifo_pos++;
		} else {
			char tmp_sql[512];
			sprintf(tmp_sql,"INSERT INTO `loginlog`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s', '%d','login failed : %%s')", p[0], p[1], p[2], p[3], t_uid, result);
			switch (result) {
				case -2:
					sprintf(tmpsql,tmp_sql,"dynamic banned.");
					break;
				case 0:
					sprintf(tmpsql,tmp_sql,"no such account.");
					break;
				case 1:
					sprintf(tmpsql,tmp_sql,"incorrect password.");
					break;
				case 3:
					sprintf(tmpsql,tmp_sql,"no such account.");
					break;
				case 4:
					sprintf(tmpsql,tmp_sql,"banned account.");
					break;
				default:
					sprintf(tmpsql,tmp_sql,"unnknown reason.");
					break;
			}
			//query
			if(mysql_query(&mysql_handle, tmpsql) ) {
					printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
			}
			if ((result == 1) && (dynamic_pass_failure_ban != 0)){	// failed password
				sprintf(tmpsql,"SELECT count(*) FROM `loginlog` WHERE `ip` = '%d.%d.%d.%d' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %d MINUTE",
				  p[0], p[1], p[2], p[3], dynamic_pass_failure_ban_time);	//how many times filed account? in one ip.
				if(mysql_query(&mysql_handle, tmpsql) ) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
				}
				//check query result
				sql_res = mysql_store_result(&mysql_handle) ;
				sql_row = mysql_fetch_row(sql_res);	//row fetching
			
				if (atoi(sql_row[0]) >= dynamic_pass_failure_ban_how_many ) {
					sprintf(tmpsql,"INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%d.%d.%d.*', NOW() , NOW() +  INTERVAL %d MINUTE ,'Password error ban: %s')", p[0], p[1], p[2], dynamic_pass_failure_ban_how_long, t_uid);
					if(mysql_query(&mysql_handle, tmpsql) ) {
							printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
					}
				}
				mysql_free_result(sql_res);
			}
			else if (result == -2){	//dynamic banned - add ip to ban list.
				sprintf(tmpsql,"INSERT INTO `ipbanlist`(`list`,`btime`,`rtime`,`reason`) VALUES ('%d.%d.%d.*', NOW() , NOW() +  INTERVAL 1 MONTH ,'Dynamic banned user id : %s')", p[0], p[1], p[2], t_uid);
				if(mysql_query(&mysql_handle, tmpsql) ) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
				}
				result = 4;
			}
			//cannot connect login failed
			WFIFOW(fd,0)=0x6a;
			WFIFOB(fd,2)=result;
			WFIFOSET(fd,23);
		}
		RFIFOSKIP(fd,(RFIFOW(fd,0)==0x64)?55:47);
		break;
	
	case 0x01db:	// request password key
		printf("Request Password key -%s\n",md5key);
		RFIFOSKIP(fd,2);
		WFIFOW(fd,0)=0x01dc;
		WFIFOW(fd,2)=4+md5keylen;
		memcpy(WFIFOP(fd,4),md5key,md5keylen);
		WFIFOSET(fd,WFIFOW(fd,2));
		break;
		
	case 0x2710:	// request Char-server connection
				if(RFIFOREST(fd)<76)
					return 0;
				{
					sprintf(tmpsql,"INSERT INTO `loginlog`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%d.%d.%d.%d', '%s@%s','100', 'charserver - %s@%d.%d.%d.%d:%d')", p[0], p[1], p[2], p[3], RFIFOP(fd, 2),RFIFOP(fd, 60),RFIFOP(fd, 60), RFIFOB(fd, 54), RFIFOB(fd, 55), RFIFOB(fd, 56), RFIFOB(fd, 57), RFIFOW(fd, 58));
	
					//query
					if(mysql_query(&mysql_handle, tmpsql) ) {
							printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
					}
					printf("server connection request %s @ %d.%d.%d.%d:%d (%d.%d.%d.%d)\n",
						RFIFOP(fd, 60), RFIFOB(fd, 54), RFIFOB(fd, 55), RFIFOB(fd, 56), RFIFOB(fd, 57), RFIFOW(fd, 58),
						p[0], p[1], p[2], p[3]);
				}
				account.userid = RFIFOP(fd, 2);
				account.passwd = RFIFOP(fd, 26);
				result = mmo_auth(&account);
				
				if(result == 100 && account.sex==2 && account.account_id<MAX_SERVERS && server_fd[account.account_id]<0){
					server[account.account_id].ip=RFIFOL(fd,54);
					server[account.account_id].port=RFIFOW(fd,58);
					memcpy(server[account.account_id].name,RFIFOP(fd,60),20);
					server[account.account_id].users=0;
					server[account.account_id].maintenance=RFIFOW(fd,82);
					server[account.account_id].new=RFIFOW(fd,84);
					server_fd[account.account_id]=fd;
					sprintf(tmpsql,"DELETE FROM `sstatus` WHERE `index`='%ld'", account.account_id);
					//query
					if(mysql_query(&mysql_handle, tmpsql) ) {
							printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
					}
					
					jstrescapecpy(t_uid,server[account.account_id].name);
					sprintf(tmpsql,"INSERT INTO `sstatus`(`index`,`name`,`user`) VALUES ( '%ld', '%s', '%d')", 
						account.account_id, server[account.account_id].name,0);
					//query
					if(mysql_query(&mysql_handle, tmpsql) ) {
							printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
					}
					WFIFOW(fd,0)=0x2711;
					WFIFOB(fd,2)=0;
					WFIFOSET(fd,3);
					session[fd]->func_parse=parse_fromchar;
				} else {
					WFIFOW(fd, 0) =0x2711;
					WFIFOB(fd, 2)=3;
					WFIFOSET(fd, 3);
				}
				RFIFOSKIP(fd, 86);
				return 0;

	case 0x7530:	// request Athena information
		WFIFOW(fd,0)=0x7531;
		WFIFOB(fd,2)=ATHENA_MAJOR_VERSION;
		WFIFOB(fd,3)=ATHENA_MINOR_VERSION;
		WFIFOB(fd,4)=ATHENA_REVISION;
		WFIFOB(fd,5)=ATHENA_RELEASE_FLAG;
		WFIFOB(fd,6)=ATHENA_OFFICIAL_FLAG;
		WFIFOB(fd,7)=ATHENA_SERVER_LOGIN;
		WFIFOW(fd,8)=ATHENA_MOD_VERSION;
		WFIFOSET(fd,10);
		RFIFOSKIP(fd,2);
		printf ("Athena version check...\n");
		break;
		
	case 0x7532:
	default:
		printf ("close session connection...\n");
		close(fd);
		session[fd]->eof=1;
		return 0;
	}
  }
  return 0;
}
//-----------------------------------------------------
//BANNED IP CHECK.
//-----------------------------------------------------
int ip_ban_check(int tid, unsigned int tick, int id, int data){
	printf("TicTac - banned IP check... ");
	
	//query
	if(mysql_query(&mysql_handle, "DELETE FROM `ipbanlist` WHERE `rtime` <= NOW()") ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	
	printf(" ok...\n");
	return 0;
}

//-----------------------------------------------------
// reading configuration
//-----------------------------------------------------
int login_config_read(const char *cfgName){
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");
	
	if(fp==NULL){
		printf("file not found: %s\n", cfgName);
		return 1;
	}
	printf ("start reading configuration...\n");
	while(fgets(line, 1020, fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;

		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		if(strcmpi(w1,"admin_pass")==0){	//leave admin_pass for special purpose.
			strcpy(admin_pass, w2);
			printf ("set admin_pass : %s\n",w2);
		}
		else if(strcmpi(w1,"login_port")==0){
			login_port=atoi(w2);
			printf ("set login_port : %s\n",w2);
		}
		else if(strcmpi(w1,"gm_account_filename")==0){ 
			strcpy(GM_account_filename,w2); 
			printf ("set gm_account_filename : %s\n",w2);
		}
		else if(strcmpi(w1,"ipban")==0){
			ipban=atoi(w2);
			printf ("set ipban : %d\n",ipban);
		}
		//account ban -> ip ban
		else if(strcmpi(w1,"dynamic_account_ban")==0){
			dynamic_account_ban=atoi(w2);
			printf ("set dynamic_account_ban : %d\n",dynamic_account_ban);
		}
		else if(strcmpi(w1,"dynamic_account_ban_class")==0){
			dynamic_account_ban_class=atoi(w2);
			printf ("set dynamic_account_ban_class : %d\n",dynamic_account_ban_class);
		}
		//dynamic password error ban
		else if(strcmpi(w1,"dynamic_pass_failure_ban")==0){
			dynamic_pass_failure_ban=atoi(w2);
			printf ("set dynamic_pass_failure_ban : %d\n",dynamic_pass_failure_ban);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_time")==0){
			dynamic_pass_failure_ban_time=atoi(w2);
			printf ("set dynamic_pass_failure_ban_time : %d\n",dynamic_pass_failure_ban_time);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_how_many")==0){
			dynamic_pass_failure_ban_how_many=atoi(w2);
			printf ("set dynamic_pass_failure_ban_how_many : %d\n",dynamic_pass_failure_ban_how_many);
		}
		else if(strcmpi(w1,"dynamic_pass_failure_ban_how_long")==0){
			dynamic_pass_failure_ban_how_long=atoi(w2);
			printf ("set dynamic_pass_failure_ban_how_long : %d\n",dynamic_pass_failure_ban_how_long);
		}
		//add for DB connection
		else if(strcmpi(w1,"db_server_ip")==0){
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
	printf ("End reading configuration...\n");
	return 0;
}

int do_init(int argc,char **argv){
	//initialize login server
	int i;
	
	//read login configue
	login_config_read( (argc>1)?argv[1]:LOGIN_CONF_NAME );
	
	//Generate Passworded Key.
	memset(md5key, 0, sizeof(md5key));
	md5keylen=rand()%4+12;

	for(i=0;i<md5keylen;i++)
		md5key[i]=rand()%255+1;

	for(i=0;i<AUTH_FIFO_SIZE;i++)
		auth_fifo[i].delflag=1;

	for(i=0;i<MAX_SERVERS;i++)
		server_fd[i]=-1;

	//server port open & binding
	make_listen_port(login_port);

	//Auth start
	mmo_auth_sqldb_init();

	//GM account setup
	read_gm_account();

	//sync account when terminating.
	//but no need when you using DBMS (mysql)
	set_termfunc(mmo_db_close);

	//set default parser as parse_login function
	set_defaultparse(parse_login);
	
	
	// ban deleter timer - 1 minute term
	printf("add interval tic (ip_ban_check)....\n");
	i=add_timer_interval(gettick()+10, ip_ban_check,0,0,60*1000);
	
	return 0;
}
