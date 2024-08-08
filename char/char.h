#include "../common/core.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/mmo.h"
#include "../common/version.h"
#include "../common/db.h"

#ifndef _CHAR_H_
#define _CHAR_H_

#define MAX_MAP_SERVERS 30

#define CHAR_CONF_NAME	"conf/char_athena.conf"
#define LOGIN_LAN_CONF_NAME	"conf/lan_support.conf"

#define UNKNOWN_CHAR_NAME "Unknown"

#define DEFAULT_AUTOSAVE_INTERVAL 300*1000

struct mmo_map_server{
  long ip;
  short port;
  int users;
  char map[MAX_MAP_PER_SERVER][16];
};
struct itemtmp {
	int flag;//checked = 1 else 0
	int id;
	short nameid;
	short amount;
	unsigned short equip;
	char identify;
	char refine;
	char attribute;
	short card[4];
};
enum {
	TABLE_INVENTORY,
	TABLE_CART,
	TABLE_STORAGE,
};
struct itemtemp{
	struct itemtmp equip[MAX_INVENTORY],notequip[MAX_INVENTORY];
};
int memitemdata_to_sql(struct itemtemp mapitem, int eqcount, int noteqcount, int char_id,int tableswitch);
int mapif_sendall(unsigned char *buf,unsigned int len);
int mapif_sendallwos(int fd,unsigned char *buf,unsigned int len);
int mapif_send(int fd,unsigned char *buf,unsigned int len);

extern int autosave_interval;

#endif

#include "inter.h"
#include "int_pet.h"
#include "int_guild.h"
#include "int_party.h"
#include "int_storage.h"
