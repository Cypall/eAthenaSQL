// $Id: itemdb.c,v 1.2 2004/02/17 04:48:07 rovert Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "itemdb.h"
#include "db.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define MAX_RANDITEM	2000

// ** ITEMDB_OVERRIDE_NAME_VERBOSE **
//   定義すると、itemdb.txtとgrfで名前が異なる場合、表示します.
//#define ITEMDB_OVERRIDE_NAME_VERBOSE	1

static struct dbt* item_db;

/*==========================================
 * DBの検索
 *------------------------------------------
 */
struct item_data* itemdb_search(int nameid)
{
	struct item_data *id;

	id=numdb_search(item_db,nameid);
	if(id) return id;

	id=malloc(sizeof(struct item_data));
	if(id==NULL){
		printf("out of memory : itemdb_search\n");
		exit(1);
	}
	memset(id,0,sizeof(struct item_data));
	numdb_insert(item_db,nameid,id);


	if(nameid>500 && nameid<600)
		id->type=0;   //heal item
	else if(nameid>600 && nameid<700)
		id->type=2;   //use item
	else if((nameid>700 && nameid<1100) ||
			(nameid>7000 && nameid<8000))
		id->type=3;   //correction
	else if(nameid>=1750 && nameid<1771)
		id->type=10;  //arrow
	else if(nameid>1100 && nameid<2000)
		id->type=4;   //weapon
	else if((nameid>2100 && nameid<3000) ||
			(nameid>5000 && nameid<6000))
		id->type=5;   //armor
	else if(nameid>4000 && nameid<5000)
		id->type=6;   //card
	else if(nameid>9000 && nameid<10000)
		id->type=7;   //egg
	else if(nameid>10000)
		id->type=8;   //petequip

	return id;
}

/*==========================================
 *
 *------------------------------------------
 */
int itemdb_isequip(int nameid)
{
	int type=itemdb_type(nameid);
	if(type==0 || type==2 || type==3 || type==6 || type==10)
		return 0;
	return 1;
}
/*==========================================
 *
 *------------------------------------------
 */
int itemdb_isequip2(struct item_data *data)
{
	if(data) {
		int type=data->type;
		if(type==0 || type==2 || type==3 || type==6 || type==10)
			return 0;
		else
			return 1;
	}
	return 0;
}



/*==========================================
 * アイテムデータベースの読み込み
 *------------------------------------------
 */
static int itemdb_readdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int nameid,j;
	char *str[32],*p,*np;
	struct item_data *id;

	fp=fopen("db/item_db.txt","r");
	if(fp==NULL){
		printf("can't read db/item_db.txt\n");
		exit(1);
	}
	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,np=p=line;j<17 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p){ *p++=0; np=p; }
		}
		if(str[0]==NULL)
			continue;
		
		nameid=atoi(str[0]);
		if(nameid<=0 || nameid>=20000)
			continue;
		ln++;

		//ID,Name,Jname,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Gender,Loc,wLV,eLV,View
		id=itemdb_search(nameid);
		memcpy(id->name,str[1],24);
		memcpy(id->jname,str[2],24);
		id->type=atoi(str[3]);

	}
	fclose(fp);
	printf("read db/item_db.txt done (count=%d)\n",ln);
	return 0;
}
static int itemdb_final(void *key,void *data,va_list ap)
{
	struct item_data *id;

	id=data;
	if(id->use_script)
		free(id->use_script);
	if(id->equip_script)
		free(id->equip_script);
	free(id);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
void do_final_itemdb(void)
{
	if(item_db){
		numdb_final(item_db,itemdb_final);
		item_db=NULL;
	}
}
int do_init_itemdb(void)
{
	item_db = numdb_init();


	itemdb_readdb();
	return 0;
}

