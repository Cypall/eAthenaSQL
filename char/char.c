// $Id: char.c,v 1.2 2004/02/17 04:48:07 rovert Exp $
// original : char2.c 2003/03/14 11:58:35 Rev.1.5
//
// original code from athena
// SQL conversion by Jioh L. Jung
//
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "char.h"
#include "strlib.h"
#include "itemdb.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

struct mmo_map_server server[MAX_MAP_SERVERS];
int server_fd[MAX_MAP_SERVERS];

int login_fd;
char userid[24];
char passwd[24];
char server_name[20];
char login_ip_str[128];
int login_ip;
int login_port = 6900;
char char_ip_str[128];
int char_ip;
int char_port = 6121;
int char_maintenance;
int char_new;
char char_txt[256];

char lan_map_ip[128];
int subneti[4];
int subnetmaski[4];

#define CHAR_STATE_WAITAUTH 0
#define CHAR_STATE_AUTHOK 1
struct char_session_data{
  int state;
  int account_id, login_id1, login_id2, sex;
  int found_char[9];
};

#define AUTH_FIFO_SIZE 256
struct {
  int account_id, char_id, login_id1, char_pos, delflag, sex;
} auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos=0;

int char_id_count=150000;
struct mmo_charstatus *char_dat;
int char_num, char_max;
int max_connect_user=0;
int autosave_interval=DEFAULT_AUTOSAVE_INTERVAL;

MYSQL cmysql_handle;
MYSQL_RES* 	sql_cres ;
MYSQL_ROW	sql_crow ;

// check for exit signal
// 0 is saving complete
// other is char_id
unsigned int save_flag = 0;

// start point (you can reset point on conf file)
struct point start_point={"new_1-1.gat", 53,111};

//=====================================================================================================
int mmo_char_tosql(int char_id, struct mmo_charstatus *p){
	int i;
	int eqcount=1;
	int noteqcount=1;

	struct itemtemp mapitem;
	save_flag = char_id;
	printf("request save char data... (%d)\n",char_id);
	
	
	
//for(testcount=1;testcount<50;testcount++){//---------------------------test count--------------------
//	printf("test count : %d\n", testcount);
//	eqcount=1;
//	noteqcount=1;
//	dbeqcount=1;
//	dbnoteqcount=1;
//-----------------------------------------------------------------------------------------------------

//=========================================map  inventory data > memory ===============================
	//map inventory data
	for(i=0;i<MAX_INVENTORY;i++){
		if(p->inventory[i].nameid>0){
			if(itemdb_isequip(p->inventory[i].nameid)==1){				
				mapitem.equip[eqcount].flag=0;
				mapitem.equip[eqcount].id = p->inventory[i].id;
				mapitem.equip[eqcount].nameid=p->inventory[i].nameid;
				mapitem.equip[eqcount].amount = p->inventory[i].amount;
				mapitem.equip[eqcount].equip = p->inventory[i].equip;
				mapitem.equip[eqcount].identify = p->inventory[i].identify;
				mapitem.equip[eqcount].refine = p->inventory[i].refine;
				mapitem.equip[eqcount].attribute = p->inventory[i].attribute;
				mapitem.equip[eqcount].card[0] = p->inventory[i].card[0];
				mapitem.equip[eqcount].card[1] = p->inventory[i].card[1];
				mapitem.equip[eqcount].card[2] = p->inventory[i].card[2];
				mapitem.equip[eqcount].card[3] = p->inventory[i].card[3];				
				eqcount++;
			}
			else if(itemdb_isequip(p->inventory[i].nameid)==0){				
				mapitem.notequip[noteqcount].flag=0;
				mapitem.notequip[noteqcount].id = p->inventory[i].id;
				mapitem.notequip[noteqcount].nameid=p->inventory[i].nameid;
				mapitem.notequip[noteqcount].amount = p->inventory[i].amount;
				mapitem.notequip[noteqcount].equip = p->inventory[i].equip;
				mapitem.notequip[noteqcount].identify = p->inventory[i].identify;
				mapitem.notequip[noteqcount].refine = p->inventory[i].refine;
				mapitem.notequip[noteqcount].attribute = p->inventory[i].attribute;
				mapitem.notequip[noteqcount].card[0] = p->inventory[i].card[0];
				mapitem.notequip[noteqcount].card[1] = p->inventory[i].card[1];
				mapitem.notequip[noteqcount].card[2] = p->inventory[i].card[2];
				mapitem.notequip[noteqcount].card[3] = p->inventory[i].card[3];				
				noteqcount++;
			}
		}
	}
	printf("- Save item data to MySQL!\n");
	memitemdata_to_sql(mapitem, eqcount, noteqcount, char_id,TABLE_INVENTORY);

//=========================================map  cart data > memory ====================================
	eqcount=1;
	noteqcount=1;
	
	//map cart data
	for(i=0;i<MAX_CART;i++){
		if(p->cart[i].nameid>0){
			if(itemdb_isequip(p->cart[i].nameid)==1){				
				mapitem.equip[eqcount].flag=0;
				mapitem.equip[eqcount].id = p->cart[i].id;
				mapitem.equip[eqcount].nameid=p->cart[i].nameid;
				mapitem.equip[eqcount].amount = p->cart[i].amount;
				mapitem.equip[eqcount].equip = p->cart[i].equip;
				mapitem.equip[eqcount].identify = p->cart[i].identify;
				mapitem.equip[eqcount].refine = p->cart[i].refine;
				mapitem.equip[eqcount].attribute = p->cart[i].attribute;
				mapitem.equip[eqcount].card[0] = p->cart[i].card[0];
				mapitem.equip[eqcount].card[1] = p->cart[i].card[1];
				mapitem.equip[eqcount].card[2] = p->cart[i].card[2];
				mapitem.equip[eqcount].card[3] = p->cart[i].card[3];				
				eqcount++;
			}
			else if(itemdb_isequip(p->cart[i].nameid)==0){				
				mapitem.notequip[noteqcount].flag=0;
				mapitem.notequip[noteqcount].id = p->cart[i].id;
				mapitem.notequip[noteqcount].nameid=p->cart[i].nameid;
				mapitem.notequip[noteqcount].amount = p->cart[i].amount;
				mapitem.notequip[noteqcount].equip = p->cart[i].equip;
				mapitem.notequip[noteqcount].identify = p->cart[i].identify;
				mapitem.notequip[noteqcount].refine = p->cart[i].refine;
				mapitem.notequip[noteqcount].attribute = p->cart[i].attribute;
				mapitem.notequip[noteqcount].card[0] = p->cart[i].card[0];
				mapitem.notequip[noteqcount].card[1] = p->cart[i].card[1];
				mapitem.notequip[noteqcount].card[2] = p->cart[i].card[2];
				mapitem.notequip[noteqcount].card[3] = p->cart[i].card[3];
				noteqcount++;
			}
		}
	}

	printf("- Save cart data to MySQL!\n");
	memitemdata_to_sql(mapitem, eqcount, noteqcount, char_id,TABLE_CART);
	
//=====================================================================================================

//}//---------------------------test count------------------------------	
	
	
	//sql query
	//`char`( `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, //9
	//`str`,`agi`,`vit`,`int`,`dex`,`luk`, //15
	//`max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, //21
	//`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`, //27
	//`hair`,`hair_color`,`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, //35
	//`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`)
	printf("- Save char data to MySQL!\n");
	sprintf(tmp_sql ,"UPDATE `char` SET `class`='%d', `base_level`='%d', `job_level`='%d',"
		"`base_exp`='%d', `job_exp`='%d', `zeny`='%d',"
		"`max_hp`='%d',`hp`='%d',`max_sp`='%d',`sp`='%d',`status_point`='%d',`skill_point`='%d',"
		"`str`='%d',`agi`='%d',`vit`='%d',`int`='%d',`dex`='%d',`luk`='%d',"
		"`option`='%d',`karma`='%d',`manner`='%d',`party_id`='%d',`guild_id`='%d',`pet_id`='%d',"
		"`hair`='%d',`hair_color`='%d',`clothes_color`='%d',`weapon`='%d',`shield`='%d',`head_top`='%d',`head_mid`='%d',`head_bottom`='%d',"
		"`last_map`='%s',`last_x`='%d',`last_y`='%d',`save_map`='%s',`save_x`='%d',`save_y`='%d' WHERE  `char_id` = '%d'",
		p->class, p->base_level, p->job_level,
		p->base_exp, p->job_exp, p->zeny,
		p->max_hp, p->hp, p->max_sp, p->sp, p->status_point, p->skill_point,
		p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
		p->option, p->karma, p->manner, p->party_id, p->guild_id, p->pet_id,
		p->hair, p->hair_color, p->clothes_color,
		p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
		p->last_point.map, p->last_point.x, p->last_point.y,
		p->save_point.map, p->save_point.x, p->save_point.y, char_id
	);
	
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
			printf("DB server Error (update `char`)- %s\n", mysql_error(&cmysql_handle) );
	}
	
	printf("- Save memo data to MySQL!\n");
	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	sprintf(tmp_sql,"DELETE FROM `memo` WHERE `char_id`='%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
			printf("DB server Error (delete `memo`)- %s\n", mysql_error(&cmysql_handle) );
	}
	
	//insert here.
	for(i=0;i<10;i++){
		if(p->memo_point[i].map[0]){
			sprintf(tmp_sql,"INSERT INTO `memo`(`char_id`,`map`,`x`,`y`) VALUES ('%d', '%s', '%d', '%d')",
				char_id, p->memo_point[i].map, p->memo_point[i].x, p->memo_point[i].y);
			if(mysql_query(&cmysql_handle, tmp_sql) )
				printf("DB server Error (insert `memo`)- %s\n", mysql_error(&cmysql_handle) );
		}
	}
	
	printf("- Save skill data to MySQL!\n");
	//`skill` (`char_id`, `id`, `lv`)
	sprintf(tmp_sql,"DELETE FROM `skill` WHERE `char_id`='%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
			printf("DB server Error (delete `skill`)- %s\n", mysql_error(&cmysql_handle) );
	}
	printf("- Insert skill \n");
	//insert here.
	for(i=0;i<MAX_SKILL;i++){
		if(p->skill[i].id){
			if (p->skill[i].id && p->skill[i].flag!=1) {
				sprintf(tmp_sql,"INSERT delayed INTO `skill`(`char_id`, `id`, `lv`) VALUES ('%d', '%d','%d')",
					char_id, p->skill[i].id, (p->skill[i].flag==0)?p->skill[i].lv:p->skill[i].flag-2);
				if(mysql_query(&cmysql_handle, tmp_sql) ) {
					printf("DB server Error (insert `skill`)- %s\n", mysql_error(&cmysql_handle) );
				}
			}
		}
	}
	
	
	printf("- Save global_reg_value data to MySQL!\n");
	//`global_reg_value` (`char_id`, `str`, `value`)
	sprintf(tmp_sql,"DELETE FROM `global_reg_value` WHERE `char_id`='%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
			printf("DB server Error (delete `global_reg_value`)- %s\n", mysql_error(&cmysql_handle) );
	}
	
	//insert here.
	for(i=0;i<p->global_reg_num;i++){
		if(p->global_reg[i].value !=0){
			sprintf(tmp_sql,"INSERT INTO `global_reg_value` (`char_id`, `str`, `value`) VALUES ('%d', '%s','%d')",
				char_id, p->global_reg[i].str, p->global_reg[i].value);
			if(mysql_query(&cmysql_handle, tmp_sql) ) {
				printf("DB server Error (insert `global_reg_value`)- %s\n", mysql_error(&cmysql_handle) );
			}
		}
	}
	printf("saving char is done... (%d)\n",char_id);
	save_flag = 0;
	
  return 0;
}



int memitemdata_to_sql(struct itemtemp mapitem, int eqcount, int noteqcount, int char_id,int tableswitch){
	//equ
	int i,j;
	int dbeqcount=1;
	int dbnoteqcount=1;
	struct itemtemp dbitem;
	char tablename[16];
	char selectoption[16];
	switch (tableswitch){
		case TABLE_INVENTORY:
			sprintf(tablename,"inventory");
			sprintf(selectoption,"char_id");
			break;
		case TABLE_CART:
			sprintf(tablename,"cart_inventory");
			sprintf(selectoption,"char_id");
			break;
		case TABLE_STORAGE:
			sprintf(tablename,"storage");
			sprintf(selectoption,"account_id");
			break;
	}
	//printf("Working Table : %s \n",tablename);
	
	//=======================================mysql database data > memory===============================================	
	
	sprintf (tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` "
		"FROM `%s` WHERE `%s`='%d'",tablename ,selectoption ,char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (select `%s` to Memory)- %s\n",tablename ,mysql_error(&cmysql_handle) );
	}
	sql_cres = mysql_store_result(&cmysql_handle);
	if (sql_cres) {
		for(i=0;(sql_crow = mysql_fetch_row(sql_cres));i++){
			if (itemdb_isequip(atoi(sql_crow[1]))==1){			
				dbitem.equip[dbeqcount].flag=0;
				dbitem.equip[dbeqcount].id = atoi(sql_crow[0]);
				dbitem.equip[dbeqcount].nameid = atoi(sql_crow[1]);
				dbitem.equip[dbeqcount].amount = atoi(sql_crow[2]);
				dbitem.equip[dbeqcount].equip = atoi(sql_crow[3]);
				dbitem.equip[dbeqcount].identify = atoi(sql_crow[4]);
				dbitem.equip[dbeqcount].refine = atoi(sql_crow[5]);
				dbitem.equip[dbeqcount].attribute = atoi(sql_crow[6]);
				dbitem.equip[dbeqcount].card[0] = atoi(sql_crow[7]);
				dbitem.equip[dbeqcount].card[1] = atoi(sql_crow[8]);
				dbitem.equip[dbeqcount].card[2] = atoi(sql_crow[9]);
				dbitem.equip[dbeqcount].card[3] = atoi(sql_crow[10]);				
				dbeqcount++;
			}else {				
				dbitem.notequip[dbnoteqcount].flag=0;
				dbitem.notequip[dbnoteqcount].id = atoi(sql_crow[0]);
				dbitem.notequip[dbnoteqcount].nameid = atoi(sql_crow[1]);
				dbitem.notequip[dbnoteqcount].amount = atoi(sql_crow[2]);
				dbitem.notequip[dbnoteqcount].equip = atoi(sql_crow[3]);
				dbitem.notequip[dbnoteqcount].identify = atoi(sql_crow[4]);
				dbitem.notequip[dbnoteqcount].refine = atoi(sql_crow[5]);
				dbitem.notequip[dbnoteqcount].attribute = atoi(sql_crow[6]);
				dbitem.notequip[dbnoteqcount].card[0] = atoi(sql_crow[7]);
				dbitem.notequip[dbnoteqcount].card[1] = atoi(sql_crow[8]);
				dbitem.notequip[dbnoteqcount].card[2] = atoi(sql_crow[9]);
				dbitem.notequip[dbnoteqcount].card[3] = atoi(sql_crow[10]);				
				dbnoteqcount++;
			}
		}
		mysql_free_result(sql_cres);
	}
	//==============================================Memory data > SQL ===============================
		//======================================Equip ITEM=======================================
	if((eqcount==1) && (dbeqcount==1)){//printf("%s Equip Empty\n",tablename);
	}//item empty
	else{
	
	for(i=1;i<eqcount;i++){
		for(j=1;j<dbeqcount;j++){
			if(mapitem.equip[i].flag==1) break;
			if(!(dbitem.equip[j].flag==1)){
				if(mapitem.equip[i].nameid==dbitem.equip[j].nameid){
					if ((mapitem.equip[i].equip==dbitem.equip[j].equip) && (mapitem.equip[i].identify==dbitem.equip[j].identify)
					&& (mapitem.equip[i].refine==dbitem.equip[j].refine) && (mapitem.equip[i].attribute==dbitem.equip[j].attribute) && (mapitem.equip[i].card[0]==dbitem.equip[j].card[0]) 
					&& (mapitem.equip[i].card[1]==dbitem.equip[j].card[1]) && (mapitem.equip[i].card[2]==dbitem.equip[j].card[2]) && (mapitem.equip[i].card[3]==dbitem.equip[j].card[3]))
					{	mapitem.equip[i].flag=1;
						dbitem.equip[j].flag=1;
						//printf("the same item : %d , equip : %d , i : %d , flag :  %d\n", mapitem.equip[i].nameid,mapitem.equip[i].equip , i, mapitem.equip[i].flag); //DEBUG-STRING
					}
					else{
						sprintf(tmp_sql,"UPDATE `%s` SET `equip`='%d', `identify`='%d', `refine`='%d',"
						"`attribute`='%d', `card0`='%d', `card1`='%d', `card2`='%d', `card3`='%d' WHERE `id`='%d' LIMIT 1\n",
						tablename, mapitem.equip[i].equip, mapitem.equip[i].identify, mapitem.equip[i].refine,mapitem.equip[i].attribute, mapitem.equip[i].card[0],
						mapitem.equip[i].card[1], mapitem.equip[i].card[2], mapitem.equip[i].card[3], dbitem.equip[j].id);
						//printf("%s\n",tmp_sql);
						if(mysql_query(&cmysql_handle, tmp_sql) )
							printf("DB server Error (UPdate `equ %s`)- %s\n", tablename, mysql_error(&cmysql_handle) );						
						mapitem.equip[i].flag=1;
						dbitem.equip[j].flag=1;
						//printf("not the same item : %d ; i : %d ; flag :  %d\n", mapitem.equip[i].nameid, i, mapitem.equip[i].flag);
					}
				}
			}
		}
	}
	
	//printf("dbeqcount = %d\n",dbeqcount);
		
	for(i=1;i<dbeqcount;i++){
		//printf("dbitem.equip[i].flag = %d , dbitem.equip[i].id = %d\n",dbitem.equip[i].flag,dbitem.equip[i].id);
		if(!(dbitem.equip[i].flag==1)){
			sprintf(tmp_sql,"DELETE from `%s` where `id`='%d' \n",tablename , dbitem.equip[i].id);
			//printf("%s", tmp_sql);
			if(mysql_query(&cmysql_handle, tmp_sql) )
				printf("DB server Error (DELETE `equ %s`)- %s\n", tablename ,mysql_error(&cmysql_handle) );
		}
	}
	for(i=1;i<eqcount;i++){
		if(!(mapitem.equip[i].flag==1)){
			sprintf(tmp_sql,"INSERT INTO `%s`(`%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)"
			" VALUES ( '%d','%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')\n",
			tablename, selectoption,  char_id, mapitem.equip[i].nameid, mapitem.equip[i].amount, mapitem.equip[i].equip, mapitem.equip[i].identify, mapitem.equip[i].refine,
			mapitem.equip[i].attribute, mapitem.equip[i].card[0], mapitem.equip[i].card[1], mapitem.equip[i].card[2], mapitem.equip[i].card[3]);
			//printf("%s", tmp_sql);
			if(mysql_query(&cmysql_handle, tmp_sql) )
				printf("DB server Error (INSERT `equ %s`)- %s\n",tablename ,mysql_error(&cmysql_handle) );
		}
	}
	

	//======================================DEBUG=================================================
	
//	gettimeofday(&tv,NULL);
//	strftime(tmpstr,24,"%Y-%m-%d %H:%M:%S",localtime(&(tv.tv_sec)));
//	printf("\n\n");
//	printf("Working Table Name : EQU %s,  Count : map %3d | db %3d \n",tablename ,eqcount ,dbeqcount);
//	printf("*********************************************************************************\n");
//	printf("======================================MAP===================Char ID %10d===\n",char_id);
//	printf("==flag ===name ===equip===ident===amoun===attri===card0===card1===card2===card3==\n");
//	for(j=1;j<eqcount;j++)
//		printf("| %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d |\n", mapitem.equip[j].flag,mapitem.equip[j].nameid, mapitem.equip[j].equip, mapitem.equip[j].identify, mapitem.equip[j].refine,mapitem.equip[j].attribute, mapitem.equip[j].card[0], mapitem.equip[j].card[1], mapitem.equip[j].card[2], mapitem.equip[j].card[3]);
//	printf("======================================DB=========================================\n");
//	printf("==flag ===name ===equip===ident===refin===attri===card0===card1===card2===card3==\n");
//	for(j=1;j<dbeqcount;j++)
 //		printf("| %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d |\n", dbitem.equip[j].flag ,dbitem.equip[j].nameid, dbitem.equip[j].equip, dbitem.equip[j].identify, dbitem.equip[j].amount,dbitem.equip[j].attribute, dbitem.equip[j].card[0], dbitem.equip[j].card[1], dbitem.equip[j].card[2], dbitem.equip[j].card[3]);
 //	printf("=================================================================================\n");
//	printf("=================================================Data Time %s===\n", tmpstr);
//	printf("=================================================================================\n");
	
	}
	
	//======================================DEBUG==================================================
	
		//=============================Not Equip ITEM==========================================
	if((noteqcount==1) && (dbnoteqcount==1)){
		//printf("%s Not Equip Empty\n",tablename);
	}//item empty
	else{
		
	for(i=1;i<noteqcount;i++){
		for(j=1;j<dbnoteqcount;j++){
			if(mapitem.notequip[i].flag==1) break;
			if(!(dbitem.notequip[j].flag==1)){
				if(mapitem.notequip[i].nameid==dbitem.notequip[j].nameid){
					if ((mapitem.notequip[i].amount==dbitem.notequip[j].amount) && (mapitem.notequip[i].equip==dbitem.notequip[j].equip) && (mapitem.notequip[i].identify==dbitem.notequip[j].identify)
					&& (mapitem.notequip[i].attribute==dbitem.notequip[j].attribute))
					{	mapitem.notequip[i].flag=1;
						dbitem.notequip[j].flag=1;
						//printf("the same item : %d ; i : %d ; flag :  %d\n", mapitem.notequip[i].nameid, i, mapitem.notequip[i].flag); //DEBUG-STRING
					}
					else{
						sprintf(tmp_sql,"UPDATE `%s` SET `amount`='%d', `equip`='%d', `identify`='%d',"
						"`attribute`='%d' WHERE `%s`='%d' AND `nameid`='%d'\n",
						tablename, mapitem.notequip[i].amount, mapitem.notequip[i].equip, mapitem.notequip[i].identify, mapitem.notequip[i].attribute,
						selectoption, char_id, mapitem.notequip[i].nameid);
						//printf("%s",tmp_sql);
						if(mysql_query(&cmysql_handle, tmp_sql) )
							printf("DB server Error (UPdate `notequ %s`)- %s\n",tablename ,mysql_error(&cmysql_handle) );
						
						mapitem.notequip[i].flag=1;
						dbitem.notequip[j].flag=1;
					}
				}
			}
		}
	}
	
	//printf("dbnoteqcount = %d\n",dbnoteqcount);
	
	for(i=1;i<dbnoteqcount;i++){
		//printf("dbitem.notequip[i].flag = %d , dbitem.notequip[i].id = %d\n",dbitem.notequip[i].flag,dbitem.notequip[i].id);
		if(!(dbitem.notequip[i].flag==1)){
			sprintf(tmp_sql,"DELETE from `%s` where `id`='%d'", tablename, dbitem.notequip[i].id);
			//printf("%s", tmp_sql);
			if(mysql_query(&cmysql_handle, tmp_sql) )
				printf("DB server Error (DELETE `notequ %s`)- %s\n", tablename ,mysql_error(&cmysql_handle) );
		}
	}
	for(i=1;i<noteqcount;i++){
		if(!(mapitem.notequip[i].flag==1)){
			sprintf(tmp_sql,"INSERT INTO `%s`( `%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)"
			" VALUES ('%d','%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')\n",
			tablename ,selectoption , char_id, mapitem.notequip[i].nameid, mapitem.notequip[i].amount, mapitem.notequip[i].equip, mapitem.notequip[i].identify, mapitem.notequip[i].refine,
			mapitem.notequip[i].attribute, mapitem.notequip[i].card[0], mapitem.notequip[i].card[1], mapitem.notequip[i].card[2], mapitem.notequip[i].card[3]);
			//printf("%s", tmp_sql);
			if(mysql_query(&cmysql_handle, tmp_sql) )
				printf("DB server Error (INSERT `notequ %s`)- %s\n", tablename, mysql_error(&cmysql_handle) );
		}
	}
	
	//======================================DEBUG=================================================
	
//	gettimeofday(&tv,NULL);
//	strftime(tmpstr,24,"%Y-%m-%d %H:%M:%S",localtime(&(tv.tv_sec)));
//	printf("\n\n");
//	printf("Working Table Name : Not EQU %s,  Count : map %3d | db %3d \n",tablename ,noteqcount ,dbnoteqcount);
//	printf("*********************************************************************************\n");
//	printf("======================================MAP===================Char ID %10d===\n",char_id);
//	printf("==flag ===name ===equip===ident===refin===attri===card0===card1===card2===card3==\n");
//	for(j=1;j<noteqcount;j++)
//		printf("| %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d |\n", mapitem.notequip[j].flag,mapitem.notequip[j].nameid, mapitem.notequip[j].equip, mapitem.notequip[j].identify, mapitem.notequip[j].refine,mapitem.notequip[j].attribute, mapitem.notequip[j].card[0], mapitem.notequip[j].card[1], mapitem.notequip[j].card[2], mapitem.notequip[j].card[3]);
//	printf("======================================DB=========================================\n");
//	printf("==flag ===name ===equip===ident===refin===attri===card0===card1===card2===card3==\n");
//	for(j=1;j<dbnoteqcount;j++)
 //		printf("| %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d |\n", dbitem.notequip[j].flag ,dbitem.notequip[j].nameid, dbitem.notequip[j].equip, dbitem.notequip[j].identify, dbitem.notequip[j].refine,dbitem.notequip[j].attribute, dbitem.notequip[j].card[0], dbitem.notequip[j].card[1], dbitem.notequip[j].card[2], dbitem.notequip[j].card[3]);
 //	printf("=================================================================================\n");
//	printf("=================================================Data Time %s===\n", tmpstr);
//	printf("=================================================================================\n");
//	
	}
	return 0;
}
//=====================================================================================================
int mmo_char_fromsql(int char_id, struct mmo_charstatus *p){
  int i,n;
	
	
	memset(p, 0, sizeof(struct mmo_charstatus));
	p->char_id = char_id;
	
	printf("request load char data... (%d)\n",p->char_id);
	
	//`char`( `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, //9
	//`str`,`agi`,`vit`,`int`,`dex`,`luk`, //15
	//`max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, //21
	//`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`, //27
	//`hair`,`hair_color`,`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, //35
	//`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`)
	//splite 2 parts. cause veeeery long SQL syntax
	
	sprintf (tmp_sql, "SELECT `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`, `max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point` FROM `char` WHERE `char_id` = '%d'",char_id);
	
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (select `char`)- %s\n", mysql_error(&cmysql_handle) );
	}
	
	sql_cres = mysql_store_result(&cmysql_handle);
	
	if (sql_cres) {
		sql_crow = mysql_fetch_row(sql_cres);
		
		p->char_id = char_id;
		p->account_id = atoi(sql_crow[1]);	p->char_num = atoi(sql_crow[2]);	strcpy(p->name, sql_crow[3]);
		
		p->class = atoi(sql_crow[4]);	p->base_level = atoi(sql_crow[5]);	p->job_level = atoi(sql_crow[6]);
		
		p->base_exp = atoi(sql_crow[7]);	p->job_exp = atoi(sql_crow[8]);	p->zeny = atoi(sql_crow[9]);
		
		p->str = atoi(sql_crow[10]);	p->agi = atoi(sql_crow[11]);	p->vit = atoi(sql_crow[12]);
		p->int_ = atoi(sql_crow[13]);	p->dex = atoi(sql_crow[14]);	p->luk = atoi(sql_crow[15]);
		
		p->max_hp = atoi(sql_crow[16]);	p->hp = atoi(sql_crow[17]);	p->max_sp = atoi(sql_crow[18]);
		p->sp = atoi(sql_crow[19]);	p->status_point = atoi(sql_crow[20]);	p->skill_point = atoi(sql_crow[21]);
		
		
		//free mysql result.
		mysql_free_result(sql_cres);
	}
	else 
		printf ("char1 - failed\n");	//Error?! ERRRRRR WHAT THAT SAY!?
	
	printf("char1 ");
	
	sprintf (tmp_sql, "SELECT `option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`hair`,`hair_color`,"
		"`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,"
		"`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y` FROM `char` WHERE `char_id` = '%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (select `char2`)- %s\n", mysql_error(&cmysql_handle) );
	}
	
	sql_cres = mysql_store_result(&cmysql_handle);
	if (sql_cres) {
		sql_crow = mysql_fetch_row(sql_cres);

		
		p->option = atoi(sql_crow[0]);	p->karma = atoi(sql_crow[1]);	p->manner = atoi(sql_crow[2]);
			p->party_id = atoi(sql_crow[3]);	p->guild_id = atoi(sql_crow[4]);	p->pet_id = atoi(sql_crow[5]);
		
		p->hair = atoi(sql_crow[6]);	p->hair_color = atoi(sql_crow[7]);	p->clothes_color = atoi(sql_crow[8]);
		p->weapon = atoi(sql_crow[9]);	p->shield = atoi(sql_crow[10]);
		p->head_top = atoi(sql_crow[11]);	p->head_mid = atoi(sql_crow[12]);	p->head_bottom = atoi(sql_crow[13]);
		strcpy(p->last_point.map,sql_crow[14]); p->last_point.x = atoi(sql_crow[15]);	p->last_point.y = atoi(sql_crow[16]);
		strcpy(p->save_point.map,sql_crow[17]); p->save_point.x = atoi(sql_crow[18]);	p->save_point.y = atoi(sql_crow[19]);
		
		//free mysql result.
		mysql_free_result(sql_cres);
	}
	else 
		printf ("char2 - failed\n");	//Error?! ERRRRRR WHAT THAT SAY!?

	printf("char2 ");

	//read memo data
	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	sprintf (tmp_sql, "SELECT `map`,`x`,`y` FROM `memo` WHERE `char_id`='%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (select `memo`)- %s\n", mysql_error(&cmysql_handle) );
	}
	sql_cres = mysql_store_result(&cmysql_handle);

	if (sql_cres) {
		for(i=0;(sql_crow = mysql_fetch_row(sql_cres));i++){
			strcpy (p->memo_point[i].map,sql_crow[0]);
			p->memo_point[i].x=atoi(sql_crow[1]);
			p->memo_point[i].y=atoi(sql_crow[2]);
			//i ++;
		}
		mysql_free_result(sql_cres);
	}
	printf("memo ");
	
	//read inventory
	//`inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
	sprintf (tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` "
		"FROM `inventory` WHERE `char_id`='%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (select `inventory`)- %s\n", mysql_error(&cmysql_handle) );
	}
	sql_cres = mysql_store_result(&cmysql_handle);
	if (sql_cres) {
		for(i=0;(sql_crow = mysql_fetch_row(sql_cres));i++){
			p->inventory[i].id = atoi(sql_crow[0]);
			p->inventory[i].nameid = atoi(sql_crow[1]);
			p->inventory[i].amount = atoi(sql_crow[2]);
			p->inventory[i].equip = atoi(sql_crow[3]);
			p->inventory[i].identify = atoi(sql_crow[4]);
			p->inventory[i].refine = atoi(sql_crow[5]);
			p->inventory[i].attribute = atoi(sql_crow[6]);
			p->inventory[i].card[0] = atoi(sql_crow[7]);
			p->inventory[i].card[1] = atoi(sql_crow[8]);
			p->inventory[i].card[2] = atoi(sql_crow[9]);
			p->inventory[i].card[3] = atoi(sql_crow[10]);
		}
		mysql_free_result(sql_cres);
	}
	printf("inventory ");

	
	//read cart.
	//`cart_inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
	sprintf (tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` "
		"FROM `cart_inventory` WHERE `char_id`='%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (select `cart_inventory`)- %s\n", mysql_error(&cmysql_handle) );
	}
	sql_cres = mysql_store_result(&cmysql_handle);
	if (sql_cres) {
		for(i=0;(sql_crow = mysql_fetch_row(sql_cres));i++){
			p->cart[i].id = atoi(sql_crow[0]);
			p->cart[i].nameid = atoi(sql_crow[1]);
			p->cart[i].amount = atoi(sql_crow[2]);
			p->cart[i].equip = atoi(sql_crow[3]);
			p->cart[i].identify = atoi(sql_crow[4]);
			p->cart[i].refine = atoi(sql_crow[5]);
			p->cart[i].attribute = atoi(sql_crow[6]);
			p->cart[i].card[0] = atoi(sql_crow[7]);
			p->cart[i].card[1] = atoi(sql_crow[8]);
			p->cart[i].card[2] = atoi(sql_crow[9]);
			p->cart[i].card[3] = atoi(sql_crow[10]);
		}
		mysql_free_result(sql_cres);
	}
	printf("cart ");
	
	//read skill
	//`skill` (`char_id`, `id`, `lv`)
	sprintf (tmp_sql, "SELECT `id`, `lv` FROM `skill` WHERE `char_id`='%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (select `skill`)- %s\n", mysql_error(&cmysql_handle) );
	}
	sql_cres = mysql_store_result(&cmysql_handle);
	if (sql_cres) {
		for(i=0;(sql_crow = mysql_fetch_row(sql_cres));i++){
			n = atoi(sql_crow[0]);
			p->skill[n].id = n; //memory!? shit!.
			p->skill[n].lv = atoi(sql_crow[1]);
		}
		mysql_free_result(sql_cres);
	}
	printf("skill ");
	
	//global_reg
	//`global_reg_value` (`char_id`, `str`, `value`)
	sprintf (tmp_sql, "SELECT `str`, `value` FROM `global_reg_value` WHERE `char_id`='%d'",char_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (select `global_reg_value`)- %s\n", mysql_error(&cmysql_handle) );
	}
	i = 0;
	sql_cres = mysql_store_result(&cmysql_handle);
	if (sql_cres) {
		for(i=0;(sql_crow = mysql_fetch_row(sql_cres));i++){
			strcpy (p->global_reg[i].str, sql_crow[0]);
			p->global_reg[i].value = atoi (sql_crow[1]);
		}
		mysql_free_result(sql_cres);
	}
	p->global_reg_num=i;
	
	printf("global_reg\n");	//ok. all data load successfuly!
	
	printf("loading char is done... (%d)\n",char_id);
	
	return 1;
}
//==========================================================================================================
int mmo_char_sql_init(void) {
	int i;
	
	printf("init start.......\n");
	// memory initialize
	// no need to set twice size in this routine. but some cause segmentation error. :P
	printf ("initializing char memory...(%d byte)\n",sizeof(struct mmo_charstatus)*2);
	char_dat = malloc(sizeof(struct mmo_charstatus)*2);

	memset(char_dat, 0, sizeof(struct mmo_charstatus)*2);
	
	// DB connection initialized
	// for char-server session only
	mysql_init(&cmysql_handle);
	printf("Connect DB server....(char server)\n");
	if(!mysql_real_connect(&cmysql_handle, db_server_ip, db_server_id, db_server_pw,
		db_server_logindb ,db_server_port, (char *)NULL, 0)) {
			// SQL connection pointer check
			printf("%s\n",mysql_error(&cmysql_handle));
			exit(1);
	}
	else {
		printf ("connect success! (char server)\n");
	}
	
	sprintf (tmp_sql , "SELECT count(*) FROM `char`");
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
			printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
	}
	sql_cres = mysql_store_result(&cmysql_handle) ;
	sql_crow = mysql_fetch_row(sql_cres);
	printf("total char data -> '%s'.......\n",sql_crow[0]);
		i = atoi (sql_crow[0]);
	mysql_free_result(sql_cres);

	if (i !=0) {
		sprintf (tmp_sql , "SELECT max(`char_id`) FROM `char`");
		if(mysql_query(&cmysql_handle, tmp_sql) ) {
				printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
		}
		sql_cres = mysql_store_result(&cmysql_handle) ;
		sql_crow = mysql_fetch_row(sql_cres);
		char_id_count = atoi (sql_crow[0]);
		
		mysql_free_result(sql_cres);
	} else
	printf("set char_id_count: %d.......\n",char_id_count);
	printf("init end.......\n");
	return 0;
}
//==========================================================================================================


int make_new_char_sql(int fd,unsigned char *dat){
	struct char_session_data *sd;
	char t_name[100];
	FILE *logfp;
	
	//aphostropy error check! - fixed!
	jstrescapecpy (t_name, dat);
	printf ("making new char...\n");

	//check stat error
	if(dat[24]+dat[25]+dat[26]+dat[27]+dat[28]+dat[29]>5*6 ||
		dat[30]>=9 ||
		dat[33]==0 || dat[33]>=20 ||
		dat[31]>=9){
			logfp=fopen("char.log","a");
			if(logfp){
				fprintf(logfp,"make new char error %d-%d %s %d, %d, %d, %d, %d, %d %d, %d" RETCODE,
					fd, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[33], dat[31]);
				printf("make new char error %d-%d %s %d, %d, %d, %d, %d, %d %d, %d" RETCODE,
					fd, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[33], dat[31]);
				fclose(logfp);
			}
			return -1;
	}

	logfp=fopen("char.log","a");
	if(logfp){
		fprintf(logfp,"make new char %d-%d %s %d, %d, %d, %d, %d, %d - %d, %d" RETCODE, fd, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[33], dat[31]);
		printf("make new char %d-%d %s %d, %d, %d, %d, %d, %d - %d, %d" RETCODE, fd, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[33], dat[31]);
		fclose(logfp);
	}

	sd=session[fd]->session_data;

	sprintf (tmp_sql, "SELECT count(*) FROM `char` WHERE `name` = '%s'",t_name);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
		return -1;
	}
	sql_cres = mysql_store_result(&cmysql_handle);
	sql_crow = mysql_fetch_row(sql_cres);
	printf ("name check result : %s\n",sql_crow[0]);
	if (atoi(sql_crow[0]) > 0) {
		mysql_free_result(sql_cres);
		return -1;
	}
	else
		mysql_free_result(sql_cres);

	// check char slot.
	sprintf (tmp_sql, "SELECT count(*) FROM `char` WHERE `account_id` = '%d' AND `char_num` = '%d'",sd->account_id, dat[30]);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
	}
	sql_cres = mysql_store_result(&cmysql_handle);
	sql_crow = mysql_fetch_row(sql_cres);
	
	printf ("slot check result : %s\n",sql_crow[0]);
	if (atoi(sql_crow[0]) > 0) {
		mysql_free_result(sql_cres);
		return -1;
	}
	else
		mysql_free_result(sql_cres);

	char_id_count++;
	
	// make new char.
	sprintf(tmp_sql,"INSERT INTO `char` (`char_id`,`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,`hair`,`hair_color`)"
		" VALUES ('%d', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d','%d', '%d','%d', '%d')",
		 char_id_count, sd->account_id , dat[30] , t_name, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29],
		(40 * (100 + dat[26])/100) , (40 * (100 + dat[26])/100 ),  (11 * (100 + dat[27])/100), (11 * (100 + dat[27])/100), dat[33], dat[31]);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (insert `char`)- %s\n", mysql_error(&cmysql_handle) );
	}
	
	//`inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
	sprintf(tmp_sql,"INSERT INTO `inventory` (`char_id`,`nameid`, `amount`, `equip`, `identify`) VALUES ('%d', '%d', '%d', '%d', '%d')",
		char_id_count, 1201,1,0x02,1); //add Knife
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (insert `inventory`)- %s\n", mysql_error(&cmysql_handle) );
	}

	sprintf(tmp_sql,"INSERT INTO `inventory` (`char_id`,`nameid`, `amount`, `equip`, `identify`) VALUES ('%d', '%d', '%d', '%d', '%d')",
		char_id_count, 2301,1,0x10,1); //add Cotton Shirt
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (insert `inventory`)- %s\n", mysql_error(&cmysql_handle) );
	}
	// respawn map and start point set
	sprintf(tmp_sql,"UPDATE `char` SET `last_map`='%s',`last_x`='%d',`last_y`='%d',`save_map`='%s',`save_x`='%d',`save_y`='%d'  WHERE  `char_id` = '%d'",
		start_point.map,start_point.x,start_point.y, start_point.map,start_point.x,start_point.y, char_id_count);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error (update `char`)- %s\n", mysql_error(&cmysql_handle) );
	}
	printf("making new char success - id:%d\n", char_id_count);
	return char_id_count;
}
//==========================================================================================================

void mmo_char_sync(void){
  printf ("mmo_char_sync() - nothing to do\n");
}
// to do
///////////////////////////
int mmo_char_sync_timer(int tid,unsigned int tick,int id,int data){
  printf ("mmo_char_sync_timer() tic - no works to do\n");
  return 0;
}

int count_users(void){
	if(login_fd>0 && session[login_fd]){
		int i, users;
		for(i=0, users=0;i<MAX_MAP_SERVERS;i++){
			if(server_fd[i]>0){
				users+=server[i].users;
			}
		}
		return users;
	}
  return 0;
}

int mmo_char_send006b(int fd, struct char_session_data *sd){
	int i, j, found_num;
// hehe. commented other. anyway there's no need to use older version.
// if use older packet version just uncomment that!
//#ifdef NEW_006b
	int offset=24;
//#else
//	int offset=4;
//#endif

	printf("mmo_char_send006b start.. (account:%d)\n",sd->account_id);
	printf ("offset -> %d...\n",offset);

	sd->state=CHAR_STATE_AUTHOK;
	
	//search char.
	sprintf (tmp_sql, "SELECT `char_id` FROM `char` WHERE `account_id` = '%d'",sd->account_id);
	if(mysql_query(&cmysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
	}
	sql_cres = mysql_store_result(&cmysql_handle);
	
	for(i=0; i < 9;i++){
		sql_crow = mysql_fetch_row(sql_cres);
		if (!sql_crow){
			printf ("no more match row(%d)\n", i);
			mysql_free_result(sql_cres);
			break;
		}
		else {
			printf("ok char find fetching(%s/%d)....\n",sql_crow[0],i);
			sd->found_char[i]= atoi(sql_crow[0]);	//sd->found_char now store char_id.
		}
	}
	
	found_num = i;
	
	printf("char fetching end (total: %d)....\n",found_num);
	
	if (i<9)
		for(j=(i+1);j<9;j++)
			sd->found_char[j]=-1;
	

	memset(WFIFOP(fd, 0), 0, offset+found_num*106);
	WFIFOW(fd, 0) =0x6b;
	WFIFOW(fd, 2) =offset+found_num*106;

	printf("get each char....\n");
	
	for( i = 0; i < found_num; i++ ) {
		
		mmo_char_fromsql(sd->found_char[i],char_dat);

		printf("ok char get(%d)....\n",sd->found_char[i]);
		memset(WFIFOP(fd, offset+(i*106)), 0x00, 106);

		//hey! no need to consume much memory.
		//just only one block!
		WFIFOL(fd, offset+(i*106)) = char_dat[0].char_id;
		WFIFOL(fd, offset+(i*106)+4) = char_dat[0].base_exp;
		WFIFOL(fd, offset+(i*106)+8) = char_dat[0].zeny;
		WFIFOL(fd, offset+(i*106)+12) = char_dat[0].job_exp;
		WFIFOL(fd, offset+(i*106)+16) = char_dat[0].job_level;

		WFIFOL(fd, offset+(i*106)+20) = 0;
		WFIFOL(fd, offset+(i*106)+24) = 0;
		WFIFOL(fd, offset+(i*106)+28) = char_dat[0].option;

		WFIFOL(fd, offset+(i*106)+32) = char_dat[0].karma;
		WFIFOL(fd, offset+(i*106)+36) = char_dat[0].manner;

		WFIFOW(fd, offset+(i*106)+40) = char_dat[0].status_point;
    		WFIFOW(fd,offset+(i*106)+42) = (char_dat[0].hp > 0x7fff)? 0x7fff:char_dat[0].hp;
    		WFIFOW(fd,offset+(i*106)+44) = (char_dat[0].max_hp > 0x7fff)? 0x7fff:char_dat[0].max_hp;
    		WFIFOW(fd,offset+(i*106)+46) = (char_dat[0].sp > 0x7fff)? 0x7fff:char_dat[0].sp;
    		WFIFOW(fd,offset+(i*106)+48) = (char_dat[0].max_sp > 0x7fff)? 0x7fff:char_dat[0].max_sp;
		WFIFOW(fd, offset+(i*106)+50) = DEFAULT_WALK_SPEED; // char_dat[0].speed;
		WFIFOW(fd, offset+(i*106)+52) = char_dat[0].class;
		WFIFOW(fd, offset+(i*106)+54) = char_dat[0].hair;
		WFIFOW(fd, offset+(i*106)+56) = char_dat[0].weapon;
		WFIFOW(fd, offset+(i*106)+58) = char_dat[0].base_level;
		WFIFOW(fd, offset+(i*106)+60) = char_dat[0].skill_point;
		WFIFOW(fd, offset+(i*106)+62) = char_dat[0].head_bottom;
		WFIFOW(fd, offset+(i*106)+64) = char_dat[0].shield;
		WFIFOW(fd, offset+(i*106)+66) = char_dat[0].head_top;
		WFIFOW(fd, offset+(i*106)+68) = char_dat[0].head_mid;
		WFIFOW(fd, offset+(i*106)+70) = char_dat[0].hair_color;
		WFIFOW(fd, offset+(i*106)+72) = char_dat[0].clothes_color;

		memcpy( WFIFOP(fd, offset+(i*106)+74), char_dat[0].name, 24 );

		WFIFOB(fd, offset+(i*106)+98) = char_dat[0].str;
		WFIFOB(fd, offset+(i*106)+99) = char_dat[0].agi;
		WFIFOB(fd, offset+(i*106)+100) = char_dat[0].vit;
		WFIFOB(fd, offset+(i*106)+101) = char_dat[0].int_;
		WFIFOB(fd, offset+(i*106)+102) = char_dat[0].dex;
		WFIFOB(fd, offset+(i*106)+103) = char_dat[0].luk;
		WFIFOB(fd, offset+(i*106)+104) = char_dat[0].char_num;
	}
	WFIFOSET(fd, WFIFOW(fd, 2));
	printf("mmo_char_send006b end..\n");
	return 0;
}

int parse_tologin(int fd){
	int i, fdc;
	struct char_session_data *sd;

	//session eof check!
	if(session[fd]->eof){
		if(fd==login_fd)
			login_fd=-1;
		close(fd);
		delete_session(fd);
		return 0;
	}
	printf("parse_tologin : %d %d %d\n", fd, RFIFOREST(fd), RFIFOW(fd, 0));
	sd=session[fd]->session_data;
	// hehe. no need to set user limite on SQL version. :P
	// but char limitation is good way to maintain server. :D
	while(RFIFOREST(fd)>=2){
		switch(RFIFOW(fd, 0)){
			case 0x2711:
				if(RFIFOREST(fd)<3)
					return 0;
				if(RFIFOB(fd, 2)){
					printf("connect login server error : %d\n", RFIFOB(fd, 2));
					return 0;
					//exit(1); //fixed for server shutdown.
				}
				RFIFOSKIP(fd, 3);
				break;
			case 0x2713:
				if(RFIFOREST(fd)<7)
					return 0;
				for(i=0;i<fd_max;i++){
					if(session[i] && (sd=session[i]->session_data)){
						if(sd->account_id==RFIFOL(fd, 2)){
							printf ("session account found - %d(fdc - %d)\n",sd->account_id,i);
							break;
						}
					}
				}
				fdc=i;
				if(fdc==fd_max){
					RFIFOSKIP(fd, 7);
					break;
				}
				printf("parse_tologin 2713 : %d\n", RFIFOB(fd, 6));
				if(RFIFOB(fd, 6)!=0){
					WFIFOW(fdc, 0) =0x6c;
					WFIFOB(fdc, 2) =0x42;
					WFIFOSET(fdc, 3);
					RFIFOSKIP(fd, 7);
					break;
				}
				if(max_connect_user > 0) {
					printf("max_connect_user > 0 : ok..\n");
					if(count_users() < max_connect_user) {
						printf("count_users() < max_connect_use : ok..\n");
						mmo_char_send006b(fdc, sd);
					}
					else {
						printf("count_users() < max_connect_use : fail..\n");
						WFIFOW(fdc, 0) =0x6c;
						WFIFOW(fdc, 2)=0;
						WFIFOSET(fdc, 3);
					}
				}
				else {
					printf("max_connect_user > 0 : fail..\n");
					mmo_char_send006b(fdc, sd);
				}
				
				RFIFOSKIP(fd, 7);
				break;
			
			/*
			case 0x2721:	// gm reply. I don't want to support this function.
				printf("0x2721:GM reply\n");
				{
				int oldacc, newacc;
				unsigned char buf[64];
				if(RFIFOREST(fd)<10)
					return 0;
				oldacc=RFIFOL(fd, 2);
				newacc=RFIFOL(fd, 6);
				RFIFOSKIP(fd, 10);
				if(newacc>0){
					for(i=0;i<char_num;i++){
						if(char_dat[i].account_id==oldacc)
							char_dat[i].account_id=newacc;
					}
				}
				WBUFW(buf,0)=0x2b0b;
				WBUFL(buf,2)=oldacc;
				WBUFL(buf,6)=newacc;
				mapif_sendall(buf,10);
//				printf("char -> map\n");
				}
				break;
			*/
			
			default:
				printf("set eof.\n");
				close(fd);
				session[fd]->eof=1;
				return 0;
		}
	}
	RFIFOFLUSH(fd);
	return 0;
}

int parse_frommap(int fd){
	int i, j;
	int id;
	
	for(id=0;id<MAX_MAP_SERVERS;id++)
		if(server_fd[id]==fd)
			break;
	if(id==MAX_MAP_SERVERS)
		session[fd]->eof=1;
	if(session[fd]->eof){
		for(i=0;i<MAX_MAP_SERVERS;i++)
			if(server_fd[i]==fd)
		server_fd[i]=-1;
		close(fd);
		delete_session(fd);
		return 0;
	}
	if (RFIFOW(fd,0) != 0x2aff)
		printf("parse_frommap : %d %d %d\n",fd,RFIFOREST(fd),RFIFOW(fd,0));
	while(RFIFOREST(fd)>=2){
		switch(RFIFOW(fd, 0)){
		// mapserver -> map parts recv.
		case 0x2afa:
			if(RFIFOREST(fd)<4 || RFIFOREST(fd)<RFIFOW(fd,2))
				return 0;
			for(i=4,j=0;i<RFIFOW(fd,2);i+=16,j++){
				memcpy(server[id].map[j],RFIFOP(fd,i),16);
				//printf("set map %d.%d : %s\n",id,j,server[id].map[j]);
			}
			i=server[id].ip;
			{
				unsigned char *p=(unsigned char *)&i;
				printf("set map %d from %d.%d.%d.%d:%d (%d maps)\n",
					id,p[0],p[1],p[2],p[3],server[id].port,j);
			}
			server[id].map[j][0]=0;
			RFIFOSKIP(fd,RFIFOW(fd,2));
			WFIFOW(fd,0)=0x2afb;
			WFIFOW(fd,2)=0;
			WFIFOSET(fd,3);
			{	// mapserver -> other map parts recv.
				unsigned char buf[16384];
				int x;
				WBUFW(buf,0)=0x2b04;
				WBUFW(buf,2)=j*16+12;
				WBUFL(buf,4)=server[id].ip;
				WBUFW(buf,8)=server[id].port;
				WBUFW(buf,10)=i;
				for(i=0;i<j;i++){
					memcpy(WBUFP(buf,12+i*16),server[id].map[i],16);
				}
				mapif_sendallwos(fd,buf,WBUFW(buf,2));
				// mapserver -> other map parts send.
				for(x=0;x<MAX_MAP_SERVERS;x++){
					if(server_fd[x]>=0 && x!=id){
						WFIFOW(fd, 0) =0x2b04;
						WFIFOL(fd, 4) =server[x].ip;
						WFIFOW(fd, 8) =server[x].port;
						for(i=0, j=0;i<MAX_MAP_PER_SERVER;i++){
							if(server[x].map[i][0]>0)
								memcpy(WFIFOP(fd,12+(j++)*16), server[x].map[i], 16);
						}
						if(j>0){
							WFIFOW(fd, 10) =j;
							WFIFOW(fd, 2) =j*16+12;
							WFIFOSET(fd, WFIFOW(fd, 2));
						}
					}
				}
			}
			break;
		// auth request
		case 0x2afc:
			if(RFIFOREST(fd)<14)
				return 0;
			printf("(AUTH request) auth_fifo search %d %d %d\n", RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10));
			for(i=0;i<AUTH_FIFO_SIZE;i++){
				if(auth_fifo[i].account_id==RFIFOL(fd,2) &&
					auth_fifo[i].char_id==RFIFOL(fd,6) &&
					auth_fifo[i].login_id1==RFIFOL(fd,10) &&
					!auth_fifo[i].delflag){
					auth_fifo[i].delflag=1;
					printf("(AUTH request) set delflag - account_id:%d/char_id:%d/login_id1:%d(fifo_id:%d)\n", RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10),i);
					break;
				}
			}
			if(i==AUTH_FIFO_SIZE){
				WFIFOW(fd,0)=0x2afe;
				WFIFOW(fd,2)=RFIFOL(fd,2);
				WFIFOB(fd,6)=0;
				WFIFOSET(fd,7);
				printf("(AUTH request) auth_fifo search error!\n");
			} else {
				printf("(AUTH request) check\n");
				WFIFOW(fd,0)=0x2afd;
				WFIFOW(fd,2)=12+sizeof(char_dat[0]);
				WFIFOL(fd,4)=RFIFOL(fd,2);
				WFIFOL(fd,8)=RFIFOL(fd,6);
				
				mmo_char_fromsql(auth_fifo[i].char_id,char_dat);
				
				char_dat[0].sex=auth_fifo[i].sex;
				memcpy(WFIFOP(fd,12),&char_dat[0],sizeof(char_dat[0]));
				WFIFOSET(fd,WFIFOW(fd,2));
				printf("(AUTH request) end\n");
			}
			RFIFOSKIP(fd,14);
			break;
		// set MAP user
		case 0x2aff:
			if(RFIFOREST(fd)<6)
				return 0;
			if (RFIFOL(fd,2) != server[id].users)
				printf ("map user: %d\n",RFIFOL(fd,2));
			server[id].users=RFIFOL(fd,2);
			RFIFOSKIP(fd,6);
			break;
		// char saving
		case 0x2b01:
			if(RFIFOREST(fd)<4 || RFIFOREST(fd)<RFIFOW(fd,2))
				return 0;
			//check account
			sprintf (tmp_sql, "SELECT count(*) FROM `char` WHERE `account_id` = '%d' AND `char_id`='%d'",RFIFOL(fd,4),RFIFOL(fd,8));
			if(mysql_query(&cmysql_handle, tmp_sql) ) {
				printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
			}
			sql_cres = mysql_store_result(&cmysql_handle);
			
			sql_crow = mysql_fetch_row(sql_cres);
			i = atoi(sql_crow[0]);
			mysql_free_result(sql_cres);
			
			if(i != 0){
				memcpy(&char_dat[0],RFIFOP(fd,12),sizeof(char_dat[0]));
				mmo_char_tosql(RFIFOL(fd,8), char_dat);
				//save to DB
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;
		// req char selection
		case 0x2b02:
			if(RFIFOREST(fd)<10)
				return 0;

			if(auth_fifo_pos>=AUTH_FIFO_SIZE){
				auth_fifo_pos=0;
			}
			printf("(charselect) auth_fifo set %d - account_id:%08x login_id1:%08x\n", auth_fifo_pos, RFIFOL(fd, 2), RFIFOL(fd, 6));
			auth_fifo[auth_fifo_pos].account_id=RFIFOL(fd, 2);
			auth_fifo[auth_fifo_pos].char_id=0;
			auth_fifo[auth_fifo_pos].login_id1=RFIFOL(fd, 6);
			auth_fifo[auth_fifo_pos].delflag=2;
			auth_fifo[auth_fifo_pos].char_pos=0;
			auth_fifo_pos++;
			
			WFIFOW(fd, 0) =0x2b03;
			WFIFOL(fd, 2) =RFIFOL(fd, 2);
			WFIFOB(fd, 6)=0;
			WFIFOSET(fd, 7);
			
			RFIFOSKIP(fd, 10);
			
			break;
		// request "change map server"
		case 0x2b05:
			if(RFIFOREST(fd)<41)
				return 0;

			if(auth_fifo_pos>=AUTH_FIFO_SIZE){
				auth_fifo_pos=0;
			}
			memcpy(WFIFOP(fd, 2), RFIFOP(fd, 2), 38);
			WFIFOW(fd, 0) =0x2b06;

			printf("(map change) auth_fifo set %d - account_id:%08x login_id1:%08x\n", auth_fifo_pos, RFIFOL(fd, 2), RFIFOL(fd, 6));
			auth_fifo[auth_fifo_pos].account_id=RFIFOL(fd, 2);
			auth_fifo[auth_fifo_pos].char_id=RFIFOL(fd, 10);
			auth_fifo[auth_fifo_pos].login_id1=RFIFOL(fd, 6);
			auth_fifo[auth_fifo_pos].delflag=0;
			auth_fifo[auth_fifo_pos].sex=RFIFOB(fd, 40);
			{
				//to do.
				int i=0;
				
				sprintf (tmp_sql, "SELECT count(*) FROM `char` WHERE `account_id` = '%d' AND `char_id`='%d'",RFIFOL(fd,2),RFIFOL(fd,10));
				if(mysql_query(&cmysql_handle, tmp_sql) ) {
					printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
				}
				sql_cres = mysql_store_result(&cmysql_handle);
				
				sql_crow = mysql_fetch_row(sql_cres);
				i = atoi(sql_crow[0]);
				mysql_free_result(sql_cres);
			
				if(i==0){
					WFIFOW(fd,6)=1;
					WFIFOSET(fd,40);
					RFIFOSKIP(fd,41);
					break;
				}
				auth_fifo[auth_fifo_pos].char_pos=auth_fifo[auth_fifo_pos].char_id;
			}
			auth_fifo_pos++;
			
			WFIFOL(fd,6)=0;
			WFIFOSET(fd,40);
			RFIFOSKIP(fd,41);
			
			break;
			
		// char name check
		case 0x2b08:
			if(RFIFOREST(fd)<6)
				return 0;
			
			sprintf (tmp_sql, "SELECT `name` FROM `char` WHERE `char_id`='%d'",RFIFOL(fd,2));
			if(mysql_query(&cmysql_handle, tmp_sql) ) {
				printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
			}
			sql_cres = mysql_store_result(&cmysql_handle);
			
			sql_crow = mysql_fetch_row(sql_cres);
				
			WFIFOW(fd,0)=0x2b09;
			WFIFOL(fd,2)=RFIFOL(fd,2);
			
			if(sql_crow)
				memcpy(WFIFOP(fd,6),sql_crow[0],24);
			else
				memcpy(WFIFOP(fd,6),UNKNOWN_CHAR_NAME,24);
			mysql_free_result(sql_cres);
			
			WFIFOSET(fd,30);
			
			RFIFOSKIP(fd,6);
			break;
		
		// I want become GM - fuck!
		/*
		case 0x2b0a:
			if(RFIFOREST(fd)<4)
				return 0;
			if(RFIFOREST(fd)<RFIFOW(fd,2))
				return 0;
			memcpy(WFIFOP(login_fd,2),RFIFOP(fd,2),RFIFOW(fd,2)-2);
			WFIFOW(login_fd,0)=0x2720;
			WFIFOSET(login_fd,RFIFOW(fd,2));
//			printf("char : change gm -> login %d %s %d\n", RFIFOL(fd, 4), RFIFOP(fd, 8), RFIFOW(fd, 2));
			RFIFOSKIP(fd, RFIFOW(fd, 2));
			break;
		*/
		default:
			// inter server - packet
			{
				int r=inter_parse_frommap(fd);
				if( r==1 )	break;		// processed
				if( r==2 )	return 0;	// need more packet
			}
			
			// no inter server packet. no char server packet -> disconnect
			close(fd);
			session[fd]->eof=1;	
			return 0;
		}
	}
	return 0;
}

int search_mapserver(char *map){
	int i, j, k;
	printf("search_mapserver %s\n", map);
	for(i=0;i<MAX_MAP_SERVERS;i++){
		if(server_fd[i]<0)
			continue;
		for(j=0;server[i].map[j][0];j++){
			//printf("%s : %s = %d\n",server[i].map[j],map,strcmp(server[i].map[j],map));
			if((k=strcmp(server[i].map[j],map))==0){
				printf("search_mapserver success %s -> %d\n", map, i);
				return i;
			}
			//printf("%s : %s = %d\n", server[i].map[j], map, k);
		}
	}
	printf("search_mapserver failed\n");
	return -1;
}

int parse_char(int fd){
	int i, ch;
	char email[40];
	struct char_session_data *sd;
	
	if(login_fd<0)
		session[fd]->eof=1;
	if(session[fd]->eof){
		if(fd==login_fd)
			login_fd=-1;
		close(fd);
		delete_session(fd);
		return 0;
	}
	if(RFIFOW(fd, 0)<30000)
		printf("parse_char : %d %d %d\n", fd, RFIFOREST(fd), RFIFOW(fd, 0));
	sd=session[fd]->session_data;
	while(RFIFOREST(fd)>=2){
		switch(RFIFOW(fd, 0)){
		case 0x65:	// request to connect
			printf("request connect - account_id:%d/login_id1:%d\n",RFIFOL(fd, 2),RFIFOL(fd, 6));
			if(RFIFOREST(fd)<17)
				return 0;
			if(sd==NULL){
				sd=session[fd]->session_data=malloc(sizeof(*sd));
				memset(sd, 0, sizeof(*sd));
			}
			sd->account_id=RFIFOL(fd, 2);
			sd->login_id1=RFIFOL(fd, 6);
			sd->login_id2=RFIFOL(fd, 10);
			sd->sex=RFIFOB(fd, 16);
			sd->state=CHAR_STATE_WAITAUTH;

			WFIFOL(fd, 0) =RFIFOL(fd, 2);
			WFIFOSET(fd, 4);

			for(i=0;i<AUTH_FIFO_SIZE;i++){
				if(auth_fifo[i].account_id==sd->account_id &&
					auth_fifo[i].login_id1==sd->login_id1 &&
					auth_fifo[i].delflag==2){
						auth_fifo[i].delflag=1;
						printf("connection request> set delflag 1(o:2)- account_id:%d/login_id1:%d(fifo_id:%d)\n", sd->account_id, sd->login_id1,i);
						break;
				}
			}
			if(i==AUTH_FIFO_SIZE){
				WFIFOW(login_fd, 0) =0x2712;
				WFIFOL(login_fd, 2) =sd->account_id;
				WFIFOL(login_fd, 6) =sd->login_id1;
				WFIFOL(login_fd, 10) =sd->login_id2;
				WFIFOB(login_fd, 14) =sd->sex;
				WFIFOSET(login_fd, 15);
			} else {
				if(max_connect_user > 0) {
					if(count_users() < max_connect_user)
						mmo_char_send006b(fd, sd);
					else {
						WFIFOW(fd, 0) =0x6c;
						WFIFOW(fd, 2)=0;
						WFIFOSET(fd, 3);
					}
				}
				else
					mmo_char_send006b(fd, sd);
			}

			RFIFOSKIP(fd, 17);
			break;
		case 0x66:	// char select
			printf("0x66> request connect - account_id:%d/char_num:%d\n",sd->account_id,RFIFOB(fd, 2));
			if(RFIFOREST(fd)<3)
				return 0;
			
			sprintf (tmp_sql, "SELECT `char_id` FROM `char` WHERE `account_id`='%d' AND `char_num`='%d'",sd->account_id, RFIFOB(fd, 2));
			if(mysql_query(&cmysql_handle, tmp_sql) ) {
				printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
			}
			sql_cres = mysql_store_result(&cmysql_handle);
			
			sql_crow = mysql_fetch_row(sql_cres);
			
			if(sql_crow)
				mmo_char_fromsql(atoi(sql_crow[0]),char_dat);
			else {
				mysql_free_result(sql_cres);
				RFIFOSKIP(fd, 3);
				break;
			}
			
				FILE *logfp;

				logfp=fopen("char.log","a");
				if(logfp){
					printf("char select %d-%d %s" RETCODE, sd->account_id, RFIFOB(fd, 2), char_dat[0].name);
					fprintf(logfp,"char select %d-%d %s" RETCODE, sd->account_id, RFIFOB(fd, 2), char_dat[0].name);
					fclose(logfp);
				}

				WFIFOW(fd, 0) =0x71;
				WFIFOL(fd, 2) =char_dat[0].char_id;
				i=search_mapserver(char_dat[0].last_point.map);
				if(i<0){
					close(fd);
					session[fd]->eof=1;
					return 0;
				}
				memcpy(WFIFOP(fd, 6), char_dat[0].last_point.map, 16);				
				WFIFOL(fd, 22) =server[i].ip;
				WFIFOW(fd, 26) =server[i].port;
				WFIFOSET(fd, 28);

				if(auth_fifo_pos>=AUTH_FIFO_SIZE){
					auth_fifo_pos=0;
				}
				printf("auth_fifo set (auth_fifo_pos:%d) - account_id:%d char_id:%d login_id1:%d\n", auth_fifo_pos, sd->account_id, char_dat[0].char_id, sd->login_id1);
				auth_fifo[auth_fifo_pos].account_id=sd->account_id;
				auth_fifo[auth_fifo_pos].char_id=char_dat[0].char_id;
				auth_fifo[auth_fifo_pos].login_id1=sd->login_id1;
				auth_fifo[auth_fifo_pos].delflag=0;
				//auth_fifo[auth_fifo_pos].char_pos=sd->found_char[ch];
				auth_fifo[auth_fifo_pos].char_pos=0;
				auth_fifo[auth_fifo_pos].sex=sd->sex;
				auth_fifo_pos++;
				printf ("0x66> end\n");
			RFIFOSKIP(fd, 3);
			break;
		case 0x67:	// make new
			printf("0x67>request make new char\n");
			if(RFIFOREST(fd)<37)
				return 0;
			i=make_new_char_sql(fd, RFIFOP(fd, 2));
			if(i<0){
				WFIFOW(fd, 0) =0x6e;
				WFIFOB(fd, 2) =0x00;
				WFIFOSET(fd, 3);
				RFIFOSKIP(fd, 37);
				break;
			}

			WFIFOW(fd, 0) =0x6d;
			memset(WFIFOP(fd, 2), 0x00, 106);
			
			mmo_char_fromsql(i,char_dat);
			i = 0;
			WFIFOL(fd, 2) = char_dat[i].char_id;
			WFIFOL(fd,2+4) = char_dat[i].base_exp;
			WFIFOL(fd,2+8) = char_dat[i].zeny;
			WFIFOL(fd,2+12) = char_dat[i].job_exp;
			WFIFOL(fd,2+16) = char_dat[i].job_level;

			WFIFOL(fd,2+28) = char_dat[i].karma;
			WFIFOL(fd,2+32) = char_dat[i].manner;

			WFIFOW(fd,2+40) = 0x30;
			WFIFOW(fd,2+42) = (char_dat[i].hp > 0x7fff)? 0x7fff:char_dat[i].hp;
			WFIFOW(fd,2+44) = (char_dat[i].max_hp > 0x7fff)? 0x7fff:char_dat[i].max_hp;
			WFIFOW(fd,2+46) = (char_dat[i].sp > 0x7fff)? 0x7fff:char_dat[i].sp;
			WFIFOW(fd,2+48) = (char_dat[i].max_sp > 0x7fff)? 0x7fff:char_dat[i].max_sp;
			WFIFOW(fd,2+50) = DEFAULT_WALK_SPEED; // char_dat[i].speed;
			WFIFOW(fd,2+52) = char_dat[i].class;
			WFIFOW(fd,2+54) = char_dat[i].hair;

			WFIFOW(fd,2+58) = char_dat[i].base_level;
			WFIFOW(fd,2+60) = char_dat[i].skill_point;

			WFIFOW(fd,2+64) = char_dat[i].shield;
			WFIFOW(fd,2+66) = char_dat[i].head_top;
			WFIFOW(fd,2+68) = char_dat[i].head_mid;
			WFIFOW(fd,2+70) = char_dat[i].hair_color;

			memcpy( WFIFOP(fd,2+74), char_dat[i].name, 24 );

			WFIFOB(fd,2+98) = char_dat[i].str;
			WFIFOB(fd,2+99) = char_dat[i].agi;
			WFIFOB(fd,2+100) = char_dat[i].vit;
			WFIFOB(fd,2+101) = char_dat[i].int_;
			WFIFOB(fd,2+102) = char_dat[i].dex;
			WFIFOB(fd,2+103) = char_dat[i].luk;
			WFIFOB(fd,2+104) = char_dat[i].char_num;

			WFIFOSET(fd, 108);
			RFIFOSKIP(fd, 37);
			//to do
			for(ch=0;ch<9;ch++) {
				if(sd->found_char[ch]==-1) {
					sd->found_char[ch]=char_dat[i].char_id;
					break;
				}
			}
		case 0x68:	// delete
			printf("0x68> request char del %d(%d)\n",sd->account_id,RFIFOL(fd, 2));
			if(RFIFOREST(fd)<46)
				return 0;
			strcpy(email,RFIFOP(fd,6));
			sprintf (tmp_sql, "SELECT `email` FROM `login` WHERE `account_id`='%d'",sd->account_id);
			
			
			if(mysql_query(&cmysql_handle, tmp_sql) ) {
				printf("DB server Error Delete Char data - %s\n", mysql_error(&cmysql_handle) );
			} 
			sql_cres = mysql_store_result(&cmysql_handle);
			if (sql_cres) {
				sql_crow = mysql_fetch_row(sql_cres);
									
				if(strcmp(email,sql_crow[0])==0){					
					mysql_free_result(sql_cres);
				}
				else{
					WFIFOW(fd, 0) =0x70;
					WFIFOB(fd, 2)=0;
					WFIFOSET(fd, 3);
					RFIFOSKIP(fd, 46);
					mysql_free_result(sql_cres);
					break;
				}				
			}
			else{
					WFIFOW(fd, 0) =0x70;
					WFIFOB(fd, 2)=0;
					WFIFOSET(fd, 3);
					RFIFOSKIP(fd, 46);
					mysql_free_result(sql_cres);
				break;
			}

			sprintf (tmp_sql, "SELECT `name` FROM `char` WHERE `char_id`='%d'",RFIFOL(fd,2));
			if(mysql_query(&cmysql_handle, tmp_sql) ) {
				printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
			}
			sql_cres = mysql_store_result(&cmysql_handle);
			
			sql_crow = mysql_fetch_row(sql_cres);
			if (sql_crow[0] !=0) {
					//delete char from SQL
					sprintf(tmp_sql,"DELETE FROM `pet` WHERE `char_id`='%d'",RFIFOL(fd, 2));
					if(mysql_query(&cmysql_handle, tmp_sql) ) {
							printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
					}
					sprintf(tmp_sql,"DELETE FROM `inventory` WHERE `char_id`='%d'",RFIFOL(fd, 2));
					if(mysql_query(&cmysql_handle, tmp_sql) ) {
							printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
					}
					sprintf(tmp_sql,"DELETE FROM `cart_inventory` WHERE `char_id`='%d'",RFIFOL(fd, 2));
					if(mysql_query(&cmysql_handle, tmp_sql) ) {
							printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
					}
					sprintf(tmp_sql,"DELETE FROM `memo` WHERE `char_id`='%d'",RFIFOL(fd, 2));
					if(mysql_query(&cmysql_handle, tmp_sql) ) {
							printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
					}
					sprintf(tmp_sql,"DELETE FROM `skill` WHERE `char_id`='%d'",RFIFOL(fd, 2));
					if(mysql_query(&cmysql_handle, tmp_sql) ) {
							printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
					}
					sprintf(tmp_sql,"DELETE FROM `char` WHERE `char_id`='%d'",RFIFOL(fd, 2));
					if(mysql_query(&cmysql_handle, tmp_sql) ) {
							printf("DB server Error - %s\n", mysql_error(&cmysql_handle) );
					}

			}
			for(i=0;i<9;i++){
				printf ("char comp: %d - %d (%d)\n",sd->found_char[i], RFIFOL(fd, 2),sd->account_id);
				if(sd->found_char[i]==RFIFOL(fd, 2)){
					for(ch=i;ch<9-1;ch++)
						sd->found_char[ch]=sd->found_char[ch+1];
					sd->found_char[8]=-1;
					break;
				}
			}
			if(i==9){//reject
				WFIFOW(fd, 0) =0x70;
				WFIFOB(fd, 2)=0;
				WFIFOSET(fd, 3);
			} else {//deleted!
				WFIFOW(fd, 0) =0x6f;
				WFIFOSET(fd, 2);
			}
			RFIFOSKIP(fd, 46);
			break;
		case 0x2af8:	// login as map-server
			if(RFIFOREST(fd)<60)
				return 0;
			WFIFOW(fd, 0) =0x2af9;
			for(i=0;i<MAX_MAP_SERVERS;i++){
				if(server_fd[i]<0)
					break;
			}
			if(i==MAX_MAP_SERVERS || strcmp(RFIFOP(fd,2),userid) || strcmp(RFIFOP(fd,26),passwd)){
				WFIFOB(fd,2)=3;
				WFIFOSET(fd,3);
				RFIFOSKIP(fd,60);
			} else {
				WFIFOB(fd,2)=0;
				session[fd]->func_parse=parse_frommap;
				server_fd[i]=fd;
				server[i].ip=RFIFOL(fd, 54);
				server[i].port=RFIFOW(fd, 58);
				server[i].users=0;
				memset(server[i].map, 0, sizeof(server[i].map));
				WFIFOSET(fd, 3);
				RFIFOSKIP(fd, 60);
				return 0;
			}
			break;
		case 0x187:	// Alive?
			if (RFIFOREST(fd) < 6) {
				return 0;
			}
			RFIFOSKIP(fd, 6);
			break;
		
		case 0x7530:	// Athena info get
			WFIFOW(fd, 0) =0x7531;
			WFIFOB(fd, 2) =ATHENA_MAJOR_VERSION;
			WFIFOB(fd, 3) =ATHENA_MINOR_VERSION;
			WFIFOB(fd, 4) =ATHENA_REVISION;
			WFIFOB(fd, 5) =ATHENA_RELEASE_FLAG;
			WFIFOB(fd, 6) =ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd, 7) =ATHENA_SERVER_INTER | ATHENA_SERVER_CHAR;
			WFIFOW(fd, 8) =ATHENA_MOD_VERSION;
			WFIFOSET(fd, 10);
			RFIFOSKIP(fd, 2);
			return 0;
		case 0x7532:	// disconnect(default also disconnect)
		default:
			close(fd);
			session[fd]->eof=1;
			return 0;
		}
	}
	RFIFOFLUSH(fd);
	return 0;
}

// MAP send all
int mapif_sendall(unsigned char *buf, unsigned int len){
	int i, c;
	for(i=0, c=0;i<MAX_MAP_SERVERS;i++){
		int fd;
		if((fd=server_fd[i])>0){
			memcpy(WFIFOP(fd, 0), buf, len);
			WFIFOSET(fd, len);
			c++;
		}
	}
	return c;
}

int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len){
	int i, c;
	for(i=0, c=0;i<MAX_MAP_SERVERS;i++){
		int fd;
		if((fd=server_fd[i])>0 && fd!=sfd){
			memcpy(WFIFOP(fd, 0), buf, len);
			WFIFOSET(fd, len);
			c++;
		}
	}
	return c;
}

int mapif_send(int fd, unsigned char *buf, unsigned int len){
	int i;
	for(i=0;i<MAX_MAP_SERVERS;i++){
		if((fd==server_fd[i])>0){
			memcpy(WFIFOP(fd, 0), buf, len);
			WFIFOSET(fd, len);
			return 1;
		}
	}
	return 0;
}

int send_users_tologin(int tid, unsigned int tick, int id, int data){
  if(login_fd>0 && session[login_fd]){
    int i, users;
    for(i=0, users=0;i<MAX_MAP_SERVERS;i++){
      if(server_fd[i]>0){
	users+=server[i].users;
      }
    }
    WFIFOW(login_fd, 0) =0x2714;
    WFIFOL(login_fd, 2) =users;
    WFIFOSET(login_fd, 6);
    for(i=0;i<MAX_MAP_SERVERS;i++){
      int fd;
      if((fd=server_fd[i])>0){
	WFIFOW(fd, 0) =0x2b00;
	WFIFOL(fd, 2) =users;
	WFIFOSET(fd, 6);
      }
    }
  }
  return 0;
}

int check_connect_login_server(int tid, unsigned int tick, int id, int data){
  if(login_fd<=0 || session[login_fd]==NULL){
    login_fd=make_connection(login_ip,login_port);
    session[login_fd]->func_parse=parse_tologin;
    WFIFOW(login_fd,0)=0x2710;
    memcpy(WFIFOP(login_fd,2),userid,24);
    memcpy(WFIFOP(login_fd,26),passwd,24);
    WFIFOL(login_fd,50)=0;
    WFIFOL(login_fd,54)=char_ip;
    WFIFOL(login_fd,58)=char_port;
    memcpy(WFIFOP(login_fd,60),server_name,20);
    WFIFOW(login_fd,82)=char_maintenance;
    WFIFOW(login_fd,84)=char_new;
    WFIFOSET(login_fd,86);
  }
  return 0;
}

void do_final(void){
	printf ("Doing final stage...\n");
	//mmo_char_sync();
	//inter_save();
	do_final_itemdb();
	//check SQL save progress.
	//wait until save char complete
	printf ("waiting until char saving complete...\n");
	do {
		sleep (0);
	}while (save_flag != 0);
	
	printf ("ok! all done...\n");
}

int char_config_read(const char *cfgName){
	int i;
	char line[1024], w1[1024], w2[1024];

	struct hostent * h_char_ip = NULL;
	struct hostent * h_login_ip = NULL;
	
	printf("reading configure: %s\n", cfgName);
	FILE *fp=fopen(cfgName,"r");
	if(fp==NULL){
		printf("file not found: %s\n",cfgName);
		exit(1);
	}

	while(fgets(line, 1020, fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"userid")==0){
			printf ("set userid : %s\n",w2);
			memcpy(userid, w2, 24);
		}
		else if(strcmpi(w1,"passwd")==0){
			printf ("set passwd : %s\n",w2);
			memcpy(passwd, w2, 24);
		} else if(strcmpi(w1,"server_name")==0){
			printf ("set server_name : '%s'\n",w2);
			memcpy(server_name, w2,16);
		} else if(strcmpi(w1,"login_ip")==0){
			strcpy(login_ip_str, w2);
			printf ("set login_ip : '%s'\n",login_ip_str);
		} else if(strcmpi(w1,"login_port")==0){
			printf ("set login_port : %s\n",w2);
			login_port=atoi(w2);
		} else if(strcmpi(w1,"char_ip")==0){
			strcpy(char_ip_str, w2);
			printf ("set char_ip : '%s'\n",char_ip_str);
		} else if(strcmpi(w1,"char_port")==0){
			printf ("set char_port : %s\n",w2);
			char_port=atoi(w2);
		} else if(strcmpi(w1,"char_maintenance")==0){
			printf ("set char_maintenance : %s\n",w2);
			char_maintenance=atoi(w2);
		} else if(strcmpi(w1,"char_new")==0){
			printf ("set char_new : %s\n",w2);
			char_new=atoi(w2);
		} else if(strcmpi(w1,"char_txt")==0){
			printf ("set char_txt : %s\n",w2);
			strcpy(char_txt, w2);
		} else if(strcmpi(w1,"max_connect_user")==0){
			printf ("set max_connect_user : %s\n",w2);
			max_connect_user=atoi(w2);
		} else if(strcmpi(w1,"autosave_time")==0){
			printf ("set autosave_time : %s\n",w2);
			autosave_interval=atoi(w2)*1000;
			if(autosave_interval <= 0)
				autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
		} else if(strcmpi(w1,"start_point")==0){
			char map[32];
			int x, y;
			if( sscanf(w2,"%[^,], %d, %d", map, &x, &y)<3 )
				continue;
			memcpy(start_point.map, map, 16);
			start_point.x=x;
			start_point.y=y;
		}
	}
	fclose(fp);
	printf("reading configure done.....\n");
		
	h_char_ip = gethostbyname (char_ip_str);
	sprintf(char_ip_str, "%d.%d.%d.%d", (unsigned char)h_char_ip->h_addr[0], (unsigned char)h_char_ip->h_addr[1],
	  (unsigned char)h_char_ip->h_addr[2], (unsigned char)h_char_ip->h_addr[3]);
//	free (h_char_ip); - ebil
	printf ("char ip (dns resolve) -> '%s'\n",char_ip_str);
	
	h_login_ip = gethostbyname (login_ip_str);
	sprintf(login_ip_str, "%d.%d.%d.%d", (unsigned char)h_login_ip->h_addr[0], (unsigned char)h_login_ip->h_addr[1],
	  (unsigned char)h_login_ip->h_addr[2], (unsigned char)h_login_ip->h_addr[3]);
//	free (h_login_ip); - ebil
	printf ("login ip (dns resolve) -> '%s'\n",login_ip_str);
	
	
	login_ip=inet_addr(login_ip_str);
	char_ip=inet_addr(char_ip_str);
	printf("set ip address.....\n");
//Read ItemDB	
	do_init_itemdb();
	return 0;
}

int do_init(int argc, char **argv){
	int i;

	for(i=0;i<MAX_MAP_SERVERS;i++)
		server_fd[i]=-1;

	char_config_read((argc<2)? CHAR_CONF_NAME:argv[1]);
	printf("charserver configuration reading done.....\n");
	
	inter_init((argc>2)?argv[2]:inter_cfgName);	// inter server ÃÊ±âÈ­
	printf("interserver configuration reading done.....\n");

	printf("start char server initializing.....\n");
	mmo_char_sql_init();
	printf("char server initializing done.....\n");

	printf("set parser -> parse_char().....\n");
	set_defaultparse(parse_char);

	printf("set terminate function -> do_final().....\n");
	set_termfunc(do_final);

	printf("open port %d.....\n",char_port);
	make_listen_port(char_port);
	
	// send ALIVE PING to login server.
	printf("add interval tic (check_connect_login_server)....\n");
	i=add_timer_interval(gettick()+10, check_connect_login_server,0,0,10*1000);

	// send USER COUNT PING to login server.	
	printf("add interval tic (send_users_tologin)....\n");
	i=add_timer_interval(gettick()+10, send_users_tologin,0,0,5*1000);

	//no need to set sync timer on SQL version.
	//printf("add interval tic (mmo_char_sync_timer)....\n");
	//i=add_timer_interval(gettick()+10, mmo_char_sync_timer, 0,0, autosave_interval);

	printf("char server init func end (now unlimited loop start!)....\n");
	return 0;
}
