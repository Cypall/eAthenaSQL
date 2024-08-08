# --------------------------------------------------------
# This file is under BSD License.
# Encoded in UTF-8
# --------------------------------------------------------
#
# athena login server - SQL dump
# - by Jioh L. Jung (ziozzang@4wish.net)
#
# database : `ragnarok`
# 
# --------------------------------------------------------
#
# = User Level =
#
# user level is ban and GM check.
# -1 : banned
# 0 : normal
# 1 or more : GM level
#
# you can handle bad user and unknown user by this flag.
# set your server sign up page default as '-1'
# that protect your server secure from bad guyz.
#
# = Login data format =
#
# login date data format
# use now() function to update lastlogin time
# and send client time as 
# sprintf("%s.%d3" , lastlogin , rand(0.999))
#
# = Password protection =
#
# and md5 is no need when you use mysql.
# cause password() and md5() functions are in mysql.
#
# = About E-mail =
#
# e-mail can be used for many purpose.
#
# = Add account =
#
# Athena-db does not support dynamic account add via
# login-server. you can add account direct to mysql.
# it's more safe and more stable, even if your server
# be a bigger one.
#
# and you can change password.
#
# * basic login server works well - but login server does not work on account making,deleting, modifing.
#   use PHP script with mysql.
# * connection log with mysql.
# * ip base ban.
#
# It's good idea to run with login server on *NIX. anyway nowadays, login server works well and stable.
# so PHP + WEB is good idea to manage login server.
#
# blah blah. sorry for my bad english. :P
#
# -->> For Korean <<--
# = 유저 레벨에 대하여 =
#
# 유저 레벨은 ban인지 일반 유저인지 GM인지 체크할수 있습니다.
# (GM체크는 미구현)
#
# -1 : banned
# 0 : normal (일반 유저)
# 1 이상 : GM
#
# 나쁜 사람의 접속이나 알지 못하는 유저는 이 flag로 막을 수 있습니다.
# 회원 가입 페이지에서 이것을 -1로 설정하면 서버 접근이 차단됩니다.
# 이것은 나쁜 사람으로부터 서버를 지킬 수 있습니다.
#
# = 로그인 시간 데이터 형식 =
#
# 로그인 데이터 형식은 업데이트 경우에 now() 함수를 사용합니다.
# 유저에게 보내는 packet은 아래와 같은 형식이 됩니다.
# sprintf("%s.%d3" , lastlogin , rand(0.999))
#
# = 비밀번호 보호 =
#
# 프로그램에 내장된 md5는 더이상 필요하지 않습니다.
# mysql을 사용하면 DB서버가 password()와 md5() 함수를 지원합니다.
#
# = e-mail =
# 전자우편 주소는 여러가지 목적으로 사용 가능합니다.
#
# = 계정의 추가 =
#
# athena-db는 더이상 유저 추가를 login서버로 가능하지 않습니다.
# 대신에 DB서버에서 직접 설정 할수 있습니다. 이것은 비밀번호 변경이나
# 유저 ban등에서 여러 장점을 가집니다.
# 서버가 커질 때에도 안정적으로 운영이 가능합니다.
#
# * 기본적인 서버는 모두 작동 - 캐릭터 생성과 삭제 및 변경은
#   login 서버로 처리하지 않습니다. mysql 연동이 되면 PHP와 같은 script를 사용해 주세요.
# * 접속 log를 DB화.
# * ip base 유저 차단이 가능.
#
# login 서버는 안정적인 *NIX base 서버로 운영 하는 것이 좋습니다. 일단 login
# 서버와 MySQL 연동은 매우 잘 작동 하므로 PHP와 연결 하면 간편하게 접속
# user 관리가 가능합니다.
# -->> For Japanese <<--
# = ユーザーレベルに対して =
#
# ユーザーレベルは banなのか一般ユーザーなのか GMなのかチェックすることができます.
# (GMチェックは美句県)
#
# -1 : banned
# 0 : normal (一般ユーザー)
# 1 以上 : GM
#
# 悪い人の接続や分からないユーザーはこの flagで阻むことができます.
# 会員加入ページでこれを -1で設定すればサーバー接近が遮られます.
# これは悪い人からサーバーを守ることができます.
#
# = ログイン時間データ形式 =
#
# ログインデータ形式はアップデート場合に now() 関数を使います.
# ユーザーに送る packetは下のような形式になります.
# sprintf("%s.%d3" , lastlogin , rand(0.999))
#
# = パスワード保護 =
#
# プログラムに内蔵した md5はこれ以上必要ではないです.
# mysqlを使えば DBサーバーが password()と md5() 関数を支援します.
#
# = e-mail =
# 電子メール住所はさまざまな目的に使用可能です.
#
# = 勘定の追加 =
#
# athena-dbはこれ以上ユーザー追加を loginサーバーにできないです.
# 代りに DBサーバーで直接設定することができます. これはパスワード
# 変更やユーザー banなどで多くの長所を持ちます.
# サーバーが大きくなる時にも安定的に運営が可能です.
#
# * 基本的なサーバーは皆作動 - キャラクター生成と削除及び変更は
#   login サーバーで処理しないです. mysql連動になったら PHPのような scriptを使ってください.
# * 接続 logを DB化.
# * ip base ユーザー遮断が可能.
#
# login サーバーは安定的な *NIX base サーバーで運営した方が良いです. 一応 login
# サーバーと MySQL 淵洞はとてもよく作動するので PHPと連結すれば手軽く接続
# user管理が可能です.
# --------------------------------------------------------
#
# Table structure `ipbanlist`
#

CREATE TABLE `ipbanlist` (
  `list` varchar(255) NOT NULL default '',
  `btime` datetime NOT NULL default '0000-00-00 00:00:00',
  `rtime` datetime NOT NULL default '0000-00-00 00:00:00',
  `reason` varchar(255) NOT NULL default ''
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure `login`
#

CREATE TABLE `login` (
  `account_id` int(10) unsigned NOT NULL auto_increment,
  `userid` varchar(24) NOT NULL default '',
  `user_pass` varchar(24) NOT NULL default '000000',
  `lastlogin` datetime NOT NULL default '0000-00-00 00:00:00',
  `sex` char(1) NOT NULL default 'M',
  `logincount` mediumint(9) NOT NULL default '0',
  `email` varchar(24) NOT NULL default 'user@athena',
  `level` tinyint(4) NOT NULL default '0',
  KEY `account_id` (`account_id`),
  KEY `userid` (`userid`)
) TYPE=MyISAM AUTO_INCREMENT=1000057 ;

# --------------------------------------------------------

#
# Table structure `loginlog`
#

CREATE TABLE `loginlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `ip` varchar(64) NOT NULL default '',
  `user` varchar(32) NOT NULL default '',
  `rcode` tinyint(4) NOT NULL default '0',
  `log` varchar(255) NOT NULL default ''
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure `ragsrvinfo`
#

CREATE TABLE `ragsrvinfo` (
  `index` int(11) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `exp` int(11) NOT NULL default '0',
  `jexp` int(11) NOT NULL default '0',
  `drop` int(11) NOT NULL default '0',
  `type` tinyint(4) NOT NULL default '0',
  `text1` varchar(255) NOT NULL default '',
  `text2` varchar(255) NOT NULL default '',
  `text3` varchar(255) NOT NULL default '',
  `text4` varchar(255) NOT NULL default ''
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure `sstatus`
#

CREATE TABLE `sstatus` (
  `index` tinyint(4) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `user` int(11) NOT NULL default '0'
) TYPE=MyISAM;
    