# Upgrade file for old database before mysql018-rev01

# Clear old data
# Caution : YOU WILL LOST ALL PARTY&GUILD!!!!!
UPDATE `char` SET `party_id`='0',`guild_id`='0';
DROP TABLE `party`;
DROP TABLE `guild`;
DROP TABLE `guild_member`;
DROP TABLE `guild_skill`;
DROP TABLE `guild_position`;
DROP TABLE `guild_alliance`;
DROP TABLE `guild_expulsion`;
DROP TABLE `guild_castle`;

# Add primary key for id
ALTER TABLE `cart_inventory` DROP COLUMN `id`;
ALTER TABLE `inventory` DROP COLUMN `id`;
ALTER TABLE `storage` DROP COLUMN `id`;
ALTER TABLE `cart_inventory` ADD `id` INT UNSIGNED NOT NULL AUTO_INCREMENT FIRST, ADD PRIMARY KEY (`id`);
ALTER TABLE `inventory` ADD `id` INT UNSIGNED NOT NULL AUTO_INCREMENT FIRST, ADD PRIMARY KEY (`id`);
ALTER TABLE `storage` ADD `id` INT UNSIGNED NOT NULL AUTO_INCREMENT FIRST, ADD PRIMARY KEY (`id`);

# Add keys to speed up query
ALTER TABLE `char` ADD INDEX (`guild_id`), ADD INDEX (`party_id`);

# Add party information
ALTER TABLE `char` DROP COLUMN `online`;
ALTER TABLE `char` ADD `online` tinyint(4) NOT NULL default '0';
CREATE TABLE `party` (
  `party_id` int(11) NOT NULL default '100',
  `name` char(100) NOT NULL default '',
  `exp` int(11) NOT NULL default '0',
  `item` int(11) NOT NULL default '0',
  `leader_id` int(11) NOT NULL default '0',
    PRIMARY KEY  (`party_id`)
) TYPE=MyISAM; 

# Add guild information
#
# Table: 'guild'
# 
CREATE TABLE `guild` (
  `guild_id` int(11) NOT NULL default '10000',
  `name` varchar(24) NOT NULL default '',
  `master` varchar(24) NOT NULL default '',
  `guild_lv` smallint(6) NOT NULL default '0',
  `connect_member` smallint(6) NOT NULL default '0',
  `max_member` smallint(6) NOT NULL default '0',
  `average_lv` smallint(6) NOT NULL default '0',
  `exp` int(11) NOT NULL default '0',
  `next_exp` int(11) NOT NULL default '0',
  `skill_point` int(11) NOT NULL default '0',
  `castle_id` int(11) NOT NULL default '-1',
  `mes1` varchar(60) NOT NULL default '',
  `mes2` varchar(120) NOT NULL default '',
  `emblem_len` int(11) NOT NULL default '0',
  `emblem_id` int(11) NOT NULL default '0',
  `emblem_data` blob NOT NULL,
  PRIMARY KEY  (`guild_id`)
) TYPE=MyISAM; 

# Table: 'guild_alliance'
# 
CREATE TABLE `guild_alliance` (
  `guild_id` int(11) NOT NULL default '0',
  `opposition` int(11) NOT NULL default '0',
  `alliance_id` int(11) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Table: 'guild_expulsion'
# 
CREATE TABLE `guild_expulsion` (
  `guild_id` int(11) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `mes` varchar(40) NOT NULL default '',
  `acc` varchar(40) NOT NULL default '',
  `account_id` int(11) NOT NULL default '0',
  `rsv1` int(11) NOT NULL default '0',
  `rsv2` int(11) NOT NULL default '0',
  `rsv3` int(11) NOT NULL default '0',
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Table: 'guild_member'
# 
CREATE TABLE `guild_member` (
  `guild_id` int(11) NOT NULL default '0',
  `account_id` int(11) NOT NULL default '0',
  `char_id` int(11) NOT NULL default '0',
  `hair` smallint(6) NOT NULL default '0',
  `hair_color` smallint(6) NOT NULL default '0',
  `gender` smallint(6) NOT NULL default '0',
  `class` smallint(6) NOT NULL default '0',
  `lv` smallint(6) NOT NULL default '0',
  `exp` int(11) NOT NULL default '0',
  `exp_payper` int(11) NOT NULL default '0',
  `online` tinyint(4) NOT NULL default '0',
  `position` smallint(6) NOT NULL default '0',
  `rsv1` int(11) NOT NULL default '0',
  `rsv2` int(11) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  KEY `guild_id` (`guild_id`),
  KEY `account_id` (`account_id`),
  KEY `char_id` (`char_id`)
) TYPE=MyISAM; 

# Table: 'guild_position'
# 
CREATE TABLE `guild_position` (
  `guild_id` int(11) NOT NULL default '0',
  `position` smallint(6) NOT NULL default '0',
  `name` varchar(24) NOT NULL default '',
  `mode` int(11) NOT NULL default '0',
  `exp_mode` int(11) NOT NULL default '0',
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Table: 'guild_skill'
# 
CREATE TABLE `guild_skill` (
  `guild_id` int(11) NOT NULL default '0',
  `id` int(11) NOT NULL default '0',
  `lv` int(11) NOT NULL default '0',
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

# Table: 'guild_castle'
# 
CREATE TABLE `guild_castle` (
  `castle_id` int(11) NOT NULL default '0',
  `guild_id` int(11) NOT NULL default '0',
  `economy` int(11) NOT NULL default '0',
  `defense` int(11) NOT NULL default '0',
  `triggerE` int(11) NOT NULL default '0',
  `triggerD` int(11) NOT NULL default '0',
  `nextTime` int(11) NOT NULL default '0',
  `payTime` int(11) NOT NULL default '0',
  `createTime` int(11) NOT NULL default '0',
  `visibleC` int(11) NOT NULL default '0',
  `visibleG0` int(11) NOT NULL default '0',
  `visibleG1` int(11) NOT NULL default '0',
  `visibleG2` int(11) NOT NULL default '0',
  `visibleG3` int(11) NOT NULL default '0',
  `visibleG4` int(11) NOT NULL default '0',
  `visibleG5` int(11) NOT NULL default '0',
  `visibleG6` int(11) NOT NULL default '0',
  `visibleG7` int(11) NOT NULL default '0',
  PRIMARY KEY  (`castle_id`),
  KEY `guild_id` (`guild_id`)
) TYPE=MyISAM; 

