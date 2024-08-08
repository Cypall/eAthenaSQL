//
// original code from athena
// SQL conversion by Jioh L. Jung
//
#include "char.h"
#include "itemdb.h"
#include <string.h>
#include <stdlib.h>

#define STORAGE_MEMINC	16

// reset by inter_config_read()
struct storage *storage=NULL;


// storage data -> DB conversion
int storage_tosql(int account_id,struct storage *p){
	int i;
	int eqcount=1;
	int noteqcount=1;
	struct itemtemp mapitem;
	for(i=0;i<MAX_INVENTORY;i++){
		if(p->storage[i].nameid>0){
			if(itemdb_isequip(p->storage[i].nameid)==1){				
				mapitem.equip[eqcount].flag=0;
				mapitem.equip[eqcount].id = p->storage[i].id;
				mapitem.equip[eqcount].nameid=p->storage[i].nameid;
				mapitem.equip[eqcount].amount = p->storage[i].amount;
				mapitem.equip[eqcount].equip = p->storage[i].equip;
				mapitem.equip[eqcount].identify = p->storage[i].identify;
				mapitem.equip[eqcount].refine = p->storage[i].refine;
				mapitem.equip[eqcount].attribute = p->storage[i].attribute;
				mapitem.equip[eqcount].card[0] = p->storage[i].card[0];
				mapitem.equip[eqcount].card[1] = p->storage[i].card[1];
				mapitem.equip[eqcount].card[2] = p->storage[i].card[2];
				mapitem.equip[eqcount].card[3] = p->storage[i].card[3];				
				eqcount++;
			}
			else if(itemdb_isequip(p->storage[i].nameid)==0){				
				mapitem.notequip[noteqcount].flag=0;
				mapitem.notequip[noteqcount].id = p->storage[i].id;
				mapitem.notequip[noteqcount].nameid=p->storage[i].nameid;
				mapitem.notequip[noteqcount].amount = p->storage[i].amount;
				mapitem.notequip[noteqcount].equip = p->storage[i].equip;
				mapitem.notequip[noteqcount].identify = p->storage[i].identify;
				mapitem.notequip[noteqcount].refine = p->storage[i].refine;
				mapitem.notequip[noteqcount].attribute = p->storage[i].attribute;
				mapitem.notequip[noteqcount].card[0] = p->storage[i].card[0];
				mapitem.notequip[noteqcount].card[1] = p->storage[i].card[1];
				mapitem.notequip[noteqcount].card[2] = p->storage[i].card[2];
				mapitem.notequip[noteqcount].card[3] = p->storage[i].card[3];				
				noteqcount++;
			}
		}
	}

	memitemdata_to_sql(mapitem, eqcount, noteqcount, account_id,TABLE_STORAGE);

	//printf ("storage dump to DB - id: %d (total: %d)\n", account_id, j);
	return 0;
}

// DB -> storage data conversion
int storage_fromsql(int account_id, struct storage *p){
	int i=0;
	
	memset(p,0,sizeof(struct storage)); //clean up memory
	p->storage_amount = 0;
	p->account_id = account_id;
	
	// storage {`account_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`card0`/`card1`/`card2`/`card3`}
	sprintf(tmp_sql,"SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`card0`,`card1`,`card2`,`card3` FROM `storage` WHERE `account_id`='%d'",account_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	
	if (sql_res) {
		while((sql_row = mysql_fetch_row(sql_res))) {	//start to fetch
			p->storage[i].id= atoi(sql_row[0]);
			p->storage[i].nameid= atoi(sql_row[1]);
			p->storage[i].amount= atoi(sql_row[2]);
			p->storage[i].equip= atoi(sql_row[3]);
			p->storage[i].identify= atoi(sql_row[4]);
			p->storage[i].refine= atoi(sql_row[5]);
			p->storage[i].attribute= atoi(sql_row[6]);
			p->storage[i].card[0]= atoi(sql_row[7]);
			p->storage[i].card[1]= atoi(sql_row[8]);
			p->storage[i].card[2]= atoi(sql_row[9]);
			p->storage[i].card[3]= atoi(sql_row[10]);
			p->storage_amount = ++i;
		}
		mysql_free_result(sql_res);
	}
	
	printf ("storage load complete from DB - id: %d (total: %d)\n", account_id, p->storage_amount);
	return 1;
}

//---------------------------------------------------------
// storage data initialize
int inter_storage_init(){
	
	//memory alloc
	printf("interserver storage memory initialize....(%d byte)\n",sizeof(struct storage));
	storage=malloc(sizeof(struct storage));
	memset(storage,0,sizeof(struct storage));
	
	return 1;
}
//---------------------------------------------------------
// storage data saving - nothing to do here.
int inter_storage_save(){
	//printf("inter_storage_save() - no works to do....\n");
	//no work to do.
	return 0;
}
//---------------------------------------------------------
// packet from map server

// recive packet about storage data
int mapif_load_storage(int fd,int account_id){
	//load from DB
	storage_fromsql(account_id, storage);
	WFIFOW(fd,0)=0x3810;
	WFIFOW(fd,2)=sizeof(struct storage)+8;
	WFIFOL(fd,4)=account_id;
	memcpy(WFIFOP(fd,8),&storage[0],sizeof(struct storage));
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
// send ack to map server which is "storage data save ok."
int mapif_save_storage_ack(int fd,int account_id){
	WFIFOW(fd,0)=0x3811;
	WFIFOL(fd,2)=account_id;
	WFIFOB(fd,6)=0;
	WFIFOSET(fd,7);
	return 0;
}

//---------------------------------------------------------
// packet from map server

// recive request about storage data
int mapif_parse_LoadStorage(int fd){
	mapif_load_storage(fd,RFIFOL(fd,2));
	return 0;
}
// storage data recive and save
int mapif_parse_SaveStorage(int fd){
	int account_id=RFIFOL(fd,4);
	int len=RFIFOW(fd,2);
	
	if(sizeof(struct storage)!=len-8){
		printf("inter storage: data size error %d %d\n",sizeof(struct storage),len-8);
	}else{
		memcpy(&storage[0],RFIFOP(fd,8),sizeof(struct storage));
		storage_tosql(account_id, storage);
		mapif_save_storage_ack(fd,account_id);
	}
	return 0;
}

int inter_storage_parse_frommap(int fd){
	switch(RFIFOW(fd,0)){
	case 0x3010: mapif_parse_LoadStorage(fd); break;
	case 0x3011: mapif_parse_SaveStorage(fd); break;
	default:
		return 0;
	}
	return 1;
}

