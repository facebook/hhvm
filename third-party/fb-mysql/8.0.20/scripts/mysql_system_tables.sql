-- Copyright (c) 2007, 2020, Oracle and/or its affiliates. All rights reserved.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License, version 2.0,
-- as published by the Free Software Foundation.
--
-- This program is also distributed with certain software (including
-- but not limited to OpenSSL) that is licensed under separate terms,
-- as designated in a particular file or component or in included license
-- documentation.  The authors of MySQL hereby grant you an additional
-- permission to link the program and your derivative works with the
-- separately licensed software that they have included with MySQL.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License, version 2.0, for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

--
-- The system tables of MySQL Server
--

set @have_innodb= (select count(engine) from information_schema.engines where engine='INNODB' and support != 'NO');
set @is_mysql_encrypted = (select ENCRYPTION from information_schema.INNODB_TABLESPACES where NAME='mysql');

-- Tables below are NOT treated as DD tables by MySQL server yet.

SET FOREIGN_KEY_CHECKS= 1;

# Added sql_mode elements and making it as SET, instead of ENUM

set default_storage_engine=InnoDB;

SET @cmd = "CREATE TABLE IF NOT EXISTS db
(
Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
Db char(64) binary DEFAULT '' NOT NULL,
User char(80) binary DEFAULT '' NOT NULL,
Select_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Insert_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Update_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Delete_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Drop_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Grant_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
References_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Index_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Alter_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_tmp_table_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Lock_tables_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Show_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Alter_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Execute_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Event_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Trigger_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
PRIMARY KEY Host (Host,Db,User), KEY User (User)
)
engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin comment='Database privileges' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


-- Remember for later if db table already existed
set @had_db_table= @@warning_count != 0;

SET @cmd = "CREATE TABLE IF NOT EXISTS user
(
Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
User char(80) binary DEFAULT '' NOT NULL,
Select_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Insert_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Update_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Delete_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Drop_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Reload_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Shutdown_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Process_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
File_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Grant_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
References_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Index_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Alter_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Show_db_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Super_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_tmp_table_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Lock_tables_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Execute_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Repl_slave_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Repl_client_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Show_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Alter_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_user_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Event_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Trigger_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_tablespace_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
ssl_type enum('','ANY','X509', 'SPECIFIED') COLLATE utf8_general_ci DEFAULT '' NOT NULL,
ssl_cipher BLOB NOT NULL,
x509_issuer BLOB NOT NULL,
x509_subject BLOB NOT NULL,
max_questions int unsigned DEFAULT 0  NOT NULL,
max_updates int unsigned DEFAULT 0  NOT NULL,
max_connections int unsigned DEFAULT 0  NOT NULL,
max_user_connections int unsigned DEFAULT 0  NOT NULL,
plugin char(64) DEFAULT 'caching_sha2_password' NOT NULL,
authentication_string TEXT,
password_expired ENUM('N', 'Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
password_last_changed timestamp NULL DEFAULT NULL,
password_lifetime smallint unsigned NULL DEFAULT NULL,
account_locked ENUM('N', 'Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Create_role_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Drop_role_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
Password_reuse_history smallint unsigned NULL DEFAULT NULL,
Password_reuse_time smallint unsigned NULL DEFAULT NULL,
Password_require_current enum('N', 'Y') COLLATE utf8_general_ci DEFAULT NULL,
User_attributes JSON DEFAULT NULL,
PRIMARY KEY Host (Host,User)
) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin comment='Users and global privileges' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


SET @cmd = "CREATE TABLE IF NOT EXISTS default_roles
(
HOST CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
USER CHAR(80) BINARY DEFAULT '' NOT NULL,
DEFAULT_ROLE_HOST CHAR(255) CHARACTER SET ASCII DEFAULT '%' NOT NULL,
DEFAULT_ROLE_USER CHAR(80) BINARY DEFAULT '' NOT NULL,
PRIMARY KEY (HOST, USER, DEFAULT_ROLE_HOST, DEFAULT_ROLE_USER)
) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin comment='Default roles' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


SET @cmd = "CREATE TABLE IF NOT EXISTS role_edges
(
FROM_HOST CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
FROM_USER CHAR(80) BINARY DEFAULT '' NOT NULL,
TO_HOST CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
TO_USER CHAR(80) BINARY DEFAULT '' NOT NULL,
WITH_ADMIN_OPTION ENUM('N', 'Y') COLLATE UTF8_GENERAL_CI DEFAULT 'N' NOT NULL,
PRIMARY KEY (FROM_HOST,FROM_USER,TO_HOST,TO_USER)
) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin comment='Role hierarchy and role grants' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


SET @cmd = "CREATE TABLE IF NOT EXISTS global_grants
(
USER CHAR(80) BINARY DEFAULT '' NOT NULL,
HOST CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
PRIV CHAR(32) COLLATE UTF8_GENERAL_CI DEFAULT '' NOT NULL,
WITH_GRANT_OPTION ENUM('N','Y') COLLATE UTF8_GENERAL_CI DEFAULT 'N' NOT NULL,
PRIMARY KEY (USER,HOST,PRIV)
) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin comment='Extended global grants' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


-- Remember for later if user table already existed
set @had_user_table= @@warning_count != 0;

SET @cmd = "CREATE TABLE IF NOT EXISTS password_history
(
  Host CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
  User CHAR(80) BINARY DEFAULT '' NOT NULL,
  Password_timestamp TIMESTAMP(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6),
  Password TEXT,
  PRIMARY KEY(Host, User, Password_timestamp DESC)
 ) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin
 comment='Password history for user accounts' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS func (  name char(64) binary DEFAULT '' NOT NULL, ret tinyint DEFAULT '0' NOT NULL, dl char(128) DEFAULT '' NOT NULL, type enum ('function','aggregate') COLLATE utf8_general_ci NOT NULL, PRIMARY KEY (name) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin   comment='User defined functions' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS plugin ( name varchar(64) DEFAULT '' NOT NULL, dl varchar(128) DEFAULT '' NOT NULL, PRIMARY KEY (name) ) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_general_ci comment='MySQL plugins' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS help_topic ( help_topic_id int unsigned not null, name char(64) not null, help_category_id smallint unsigned not null, description text not null, example text not null, url text not null, primary key (help_topic_id), unique index (name) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8 comment='help topics' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS help_category ( help_category_id smallint unsigned not null, name  char(64) not null, parent_category_id smallint unsigned null, url text not null, primary key (help_category_id), unique index (name) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8 comment='help categories' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS help_relation ( help_topic_id int unsigned not null, help_keyword_id  int unsigned not null, primary key (help_keyword_id, help_topic_id) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8 comment='keyword-topic relation' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS servers ( Server_name char(64) NOT NULL DEFAULT '', Host char(255) CHARACTER SET ASCII NOT NULL DEFAULT '', Db char(64) NOT NULL DEFAULT '', Username char(64) NOT NULL DEFAULT '', Password char(64) NOT NULL DEFAULT '', Port INT NOT NULL DEFAULT '0', Socket char(64) NOT NULL DEFAULT '', Wrapper char(64) NOT NULL DEFAULT '', Owner char(64) NOT NULL DEFAULT '', PRIMARY KEY (Server_name)) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 comment='MySQL Foreign Servers table' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS tables_priv ( Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL, Db char(64) binary DEFAULT '' NOT NULL, User char(80) binary DEFAULT '' NOT NULL, Table_name char(64) binary DEFAULT '' NOT NULL, Grantor varchar(288) DEFAULT '' NOT NULL, Timestamp timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, Table_priv set('Select','Insert','Update','Delete','Create','Drop','Grant','References','Index','Alter','Create View','Show view','Trigger') COLLATE utf8_general_ci DEFAULT '' NOT NULL, Column_priv set('Select','Insert','Update','References') COLLATE utf8_general_ci DEFAULT '' NOT NULL, PRIMARY KEY (Host,Db,User,Table_name), KEY Grantor (Grantor) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8   engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin   comment='Table privileges' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS columns_priv ( Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL, Db char(64) binary DEFAULT '' NOT NULL, User char(80) binary DEFAULT '' NOT NULL, Table_name char(64) binary DEFAULT '' NOT NULL, Column_name char(64) binary DEFAULT '' NOT NULL, Timestamp timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, Column_priv set('Select','Insert','Update','References') COLLATE utf8_general_ci DEFAULT '' NOT NULL, PRIMARY KEY (Host,Db,User,Table_name,Column_name) ) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin   comment='Column privileges' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS help_keyword (   help_keyword_id  int unsigned not null, name char(64) not null, primary key (help_keyword_id), unique index (name) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8 comment='help keywords' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS time_zone_name (   Name char(64) NOT NULL, Time_zone_id int unsigned NOT NULL, PRIMARY KEY Name (Name) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8   comment='Time zone names' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS time_zone (   Time_zone_id int unsigned NOT NULL auto_increment, Use_leap_seconds enum('Y','N') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL, PRIMARY KEY TzId (Time_zone_id) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8   comment='Time zones' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS time_zone_transition (   Time_zone_id int unsigned NOT NULL, Transition_time bigint signed NOT NULL, Transition_type_id int unsigned NOT NULL, PRIMARY KEY TzIdTranTime (Time_zone_id, Transition_time) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8   comment='Time zone transitions' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS time_zone_transition_type (   Time_zone_id int unsigned NOT NULL, Transition_type_id int unsigned NOT NULL, Offset int signed DEFAULT 0 NOT NULL, Is_DST tinyint unsigned DEFAULT 0 NOT NULL, Abbreviation char(8) DEFAULT '' NOT NULL, PRIMARY KEY TzIdTrTId (Time_zone_id, Transition_type_id) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8   comment='Time zone transition types' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS time_zone_leap_second (   Transition_time bigint signed NOT NULL, Correction int signed NOT NULL, PRIMARY KEY TranTime (Transition_time) ) engine=INNODB STATS_PERSISTENT=0 CHARACTER SET utf8   comment='Leap seconds information for time zones' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;



SET @cmd = "CREATE TABLE IF NOT EXISTS procs_priv ( Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL, Db char(64) binary DEFAULT '' NOT NULL, User char(80) binary DEFAULT '' NOT NULL, Routine_name char(64) COLLATE utf8_general_ci DEFAULT '' NOT NULL, Routine_type enum('FUNCTION','PROCEDURE') NOT NULL, Grantor varchar(288) DEFAULT '' NOT NULL, Proc_priv set('Execute','Alter Routine','Grant') COLLATE utf8_general_ci DEFAULT '' NOT NULL, Timestamp timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, PRIMARY KEY (Host,Db,User,Routine_name,Routine_type), KEY Grantor (Grantor) ) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin   comment='Procedure privileges' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


-- bug#92988: Temporarily turn off sql_require_primary_key for the
-- next 2 tables as this is not necessary as they do not replicate.
SET @old_sql_require_primary_key = @@session.sql_require_primary_key;
SET @@session.sql_require_primary_key = 0;

-- Create general_log
CREATE TABLE IF NOT EXISTS general_log (event_time TIMESTAMP(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6), user_host MEDIUMTEXT NOT NULL, thread_id BIGINT UNSIGNED NOT NULL, server_id INTEGER UNSIGNED NOT NULL, command_type VARCHAR(64) NOT NULL, argument MEDIUMBLOB NOT NULL) engine=CSV CHARACTER SET utf8 comment="General log";

-- Create slow_log
CREATE TABLE IF NOT EXISTS slow_log (start_time TIMESTAMP(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6), user_host MEDIUMTEXT NOT NULL, query_time TIME(6) NOT NULL, lock_time TIME(6) NOT NULL, rows_sent INTEGER NOT NULL, rows_examined INTEGER NOT NULL, db VARCHAR(512) NOT NULL, last_insert_id INTEGER NOT NULL, insert_id INTEGER NOT NULL, server_id INTEGER UNSIGNED NOT NULL, sql_text MEDIUMBLOB NOT NULL, thread_id BIGINT UNSIGNED NOT NULL) engine=CSV CHARACTER SET utf8 comment="Slow log";

SET @@session.sql_require_primary_key = @old_sql_require_primary_key;

SET @cmd = "CREATE TABLE IF NOT EXISTS component ( component_id int unsigned NOT NULL AUTO_INCREMENT, component_group_id int unsigned NOT NULL, component_urn text NOT NULL, PRIMARY KEY (component_id)) engine=INNODB DEFAULT CHARSET=utf8 COMMENT 'Components' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


SET @cmd="CREATE TABLE IF NOT EXISTS slave_relay_log_info (
  Number_of_lines INTEGER UNSIGNED NOT NULL COMMENT 'Number of lines in the file or rows in the table. Used to version table definitions.',
  Relay_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The name of the current relay log file.',
  Relay_log_pos BIGINT UNSIGNED COMMENT 'The relay log position of the last executed event.',
  Master_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The name of the master binary log file from which the events in the relay log file were read.',
  Master_log_pos BIGINT UNSIGNED COMMENT 'The master log position of the last executed event.',
  Sql_delay INTEGER COMMENT 'The number of seconds that the slave must lag behind the master.',
  Number_of_workers INTEGER UNSIGNED,
  Id INTEGER UNSIGNED COMMENT 'Internal Id that uniquely identifies this record.',
  Channel_name CHAR(64) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL COMMENT 'The channel on which the slave is connected to a source. Used in Multisource Replication',
  Privilege_checks_username CHAR(32) COLLATE utf8_bin DEFAULT NULL COMMENT 'Username part of PRIVILEGE_CHECKS_USER.',
  Privilege_checks_hostname CHAR(255) CHARACTER SET ascii COLLATE ascii_general_ci DEFAULT NULL COMMENT 'Hostname part of PRIVILEGE_CHECKS_USER.',
  Require_row_format BOOLEAN NOT NULL COMMENT 'Indicates whether the channel shall only accept row based events.',
  Require_table_primary_key_check ENUM('STREAM','ON','OFF') NOT NULL DEFAULT 'STREAM' COMMENT 'Indicates what is the channel policy regarding tables having primary keys on create and alter table queries',
  PRIMARY KEY(Channel_name)) DEFAULT CHARSET=utf8 STATS_PERSISTENT=0 COMMENT 'Relay Log Information'";

SET @str=IF(@have_innodb <> 0, CONCAT(@cmd, ' ENGINE= INNODB ROW_FORMAT=DYNAMIC TABLESPACE=mysql ENCRYPTION=\'', @is_mysql_encrypted,'\''), CONCAT(@cmd, ' ENGINE= MYISAM'));
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd= "CREATE TABLE IF NOT EXISTS slave_master_info (
  Number_of_lines INTEGER UNSIGNED NOT NULL COMMENT 'Number of lines in the file.', 
  Master_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin NOT NULL COMMENT 'The name of the master binary log currently being read from the master.', 
  Master_log_pos BIGINT UNSIGNED NOT NULL COMMENT 'The master log position of the last read event.', 
  Host CHAR(255) CHARACTER SET ASCII COMMENT 'The host name of the master.',
  User_name TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The user name used to connect to the master.', 
  User_password TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The password used to connect to the master.', 
  Port INTEGER UNSIGNED NOT NULL COMMENT 'The network port used to connect to the master.', 
  Connect_retry INTEGER UNSIGNED NOT NULL COMMENT 'The period (in seconds) that the slave will wait before trying to reconnect to the master.', 
  Enabled_ssl BOOLEAN NOT NULL COMMENT 'Indicates whether the server supports SSL connections.', 
  Ssl_ca TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The file used for the Certificate Authority (CA) certificate.', 
  Ssl_capath TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The path to the Certificate Authority (CA) certificates.', 
  Ssl_cert TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The name of the SSL certificate file.', 
  Ssl_cipher TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The name of the cipher in use for the SSL connection.', 
  Ssl_key TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The name of the SSL key file.', 
  Ssl_verify_server_cert BOOLEAN NOT NULL COMMENT 'Whether to verify the server certificate.', 
  Heartbeat FLOAT NOT NULL COMMENT '', 
  Bind TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'Displays which interface is employed when connecting to the MySQL server', 
  Ignored_server_ids TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The number of server IDs to be ignored, followed by the actual server IDs', 
  Uuid TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The master server uuid.', 
  Retry_count BIGINT UNSIGNED NOT NULL COMMENT 'Number of reconnect attempts, to the master, before giving up.', 
  Ssl_crl TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The file used for the Certificate Revocation List (CRL)', 
  Ssl_crlpath TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The path used for Certificate Revocation List (CRL) files', 
  Enabled_auto_position BOOLEAN NOT NULL COMMENT 'Indicates whether GTIDs will be used to retrieve events from the master.',
  Channel_name CHAR(64) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL COMMENT 'The channel on which the slave is connected to a source. Used in Multisource Replication',
  Tls_version TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'Tls version',
  Public_key_path TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The file containing public key of master server.',
  Get_public_key BOOLEAN NOT NULL COMMENT 'Preference to get public key from master.',
  Network_namespace TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'Network namespace used for communication with the master server.',
  Master_compression_algorithm CHAR(64) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL COMMENT 'Compression algorithm supported for data transfer between master and slave.',
  Master_zstd_compression_level INTEGER UNSIGNED NOT NULL COMMENT 'Compression level associated with zstd compression algorithm.',
  Tls_ciphersuites TEXT CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL COMMENT 'Ciphersuites used for TLS 1.3 communication with the master server.',
  PRIMARY KEY(Channel_name)) DEFAULT CHARSET=utf8 STATS_PERSISTENT=0 COMMENT 'Master Information'";

SET @str=IF(@have_innodb <> 0, CONCAT(@cmd, ' ENGINE= INNODB ROW_FORMAT=DYNAMIC TABLESPACE=mysql ENCRYPTION=\'', @is_mysql_encrypted,'\''), CONCAT(@cmd, ' ENGINE= MYISAM'));
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd= "CREATE TABLE IF NOT EXISTS slave_worker_info (
  Id INTEGER UNSIGNED NOT NULL,
  Relay_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  Relay_log_pos BIGINT UNSIGNED NOT NULL,
  Master_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  Master_log_pos BIGINT UNSIGNED NOT NULL,
  Checkpoint_relay_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  Checkpoint_relay_log_pos BIGINT UNSIGNED NOT NULL,
  Checkpoint_master_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  Checkpoint_master_log_pos BIGINT UNSIGNED NOT NULL,
  Checkpoint_seqno INT UNSIGNED NOT NULL,
  Checkpoint_group_size INTEGER UNSIGNED NOT NULL,
  Checkpoint_group_bitmap BLOB NOT NULL,
  Channel_name CHAR(64) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL COMMENT 'The channel on which the slave is connected to a source. Used in Multisource Replication',
  PRIMARY KEY(Channel_name, Id)) DEFAULT CHARSET=utf8 STATS_PERSISTENT=0 COMMENT 'Worker Information'";

SET @str=IF(@have_innodb <> 0, CONCAT(@cmd, ' ENGINE= INNODB ROW_FORMAT=DYNAMIC TABLESPACE=mysql ENCRYPTION=\'', @is_mysql_encrypted,'\''), CONCAT(@cmd, ' ENGINE= MYISAM'));
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd= "CREATE TABLE IF NOT EXISTS gtid_executed (
    source_uuid CHAR(36) NOT NULL COMMENT 'uuid of the source where the transaction was originally executed.',
    interval_start BIGINT NOT NULL COMMENT 'First number of interval.',
    interval_end BIGINT NOT NULL COMMENT 'Last number of interval.',
    PRIMARY KEY(source_uuid, interval_start))";

SET @str=IF(@have_innodb <> 0, CONCAT(@cmd, ' ENGINE= INNODB ROW_FORMAT=DYNAMIC TABLESPACE=mysql ENCRYPTION=\'', @is_mysql_encrypted,'\''), CONCAT(@cmd, ' ENGINE= MYISAM'));
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

--
-- Optimizer Cost Model configuration
-- (Note: Column definition for default_value needs to be updated when a
--        default value is changed).
--

-- Server cost constants

SET @cmd = "CREATE TABLE IF NOT EXISTS server_cost (
  cost_name   VARCHAR(64) NOT NULL,
  cost_value  FLOAT DEFAULT NULL,
  last_update TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  comment     VARCHAR(1024) DEFAULT NULL,
  default_value FLOAT GENERATED ALWAYS AS
    (CASE cost_name
       WHEN 'disk_temptable_create_cost' THEN 20.0
       WHEN 'disk_temptable_row_cost' THEN 0.5
       WHEN 'key_compare_cost' THEN 0.05
       WHEN 'memory_temptable_create_cost' THEN 1.0
       WHEN 'memory_temptable_row_cost' THEN 0.1
       WHEN 'row_evaluate_cost' THEN 0.1
       ELSE NULL
     END) VIRTUAL,
  PRIMARY KEY (cost_name)
) ENGINE=InnoDB CHARACTER SET=utf8 COLLATE=utf8_general_ci STATS_PERSISTENT=0 ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


INSERT IGNORE INTO server_cost(cost_name) VALUES
  ("row_evaluate_cost"), ("key_compare_cost"),
  ("memory_temptable_create_cost"), ("memory_temptable_row_cost"),
  ("disk_temptable_create_cost"), ("disk_temptable_row_cost");

-- Engine cost constants

SET @cmd = "CREATE TABLE IF NOT EXISTS engine_cost (
  engine_name VARCHAR(64) NOT NULL,
  device_type INTEGER NOT NULL,
  cost_name   VARCHAR(64) NOT NULL,
  cost_value  FLOAT DEFAULT NULL,
  last_update TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  comment     VARCHAR(1024) DEFAULT NULL,
  default_value FLOAT GENERATED ALWAYS AS
    (CASE cost_name
       WHEN 'io_block_read_cost' THEN 1.0
       WHEN 'memory_block_read_cost' THEN 0.25
       ELSE NULL
     END) VIRTUAL,
  PRIMARY KEY (cost_name, engine_name, device_type)
) ENGINE=InnoDB CHARACTER SET=utf8 COLLATE=utf8_general_ci STATS_PERSISTENT=0 ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


INSERT IGNORE INTO engine_cost(engine_name, device_type, cost_name) VALUES
  ("default", 0, "memory_block_read_cost"),
  ("default", 0, "io_block_read_cost");


SET @cmd = "CREATE TABLE IF NOT EXISTS proxies_priv (Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL, User char(80) binary DEFAULT '' NOT NULL, Proxied_host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL, Proxied_user char(80) binary DEFAULT '' NOT NULL, With_grant BOOL DEFAULT 0 NOT NULL, Grantor varchar(288) DEFAULT '' NOT NULL, Timestamp timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, PRIMARY KEY Host (Host,User,Proxied_host,Proxied_user), KEY Grantor (Grantor) ) engine=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin comment='User proxy privileges' ROW_FORMAT=DYNAMIC TABLESPACE=mysql";
SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;


-- Remember for later if proxies_priv table already existed
set @had_proxies_priv_table= @@warning_count != 0;


--
-- Only create the ndb_binlog_index table if the server is built with ndb.
-- Create this table last among the tables in the mysql schema to make it
-- easier to keep tests agnostic wrt. the existence of this table.
--
SET @have_ndb= (select count(engine) from information_schema.engines where engine='ndbcluster');
SET @cmd="CREATE TABLE IF NOT EXISTS ndb_binlog_index (
  Position BIGINT UNSIGNED NOT NULL,
  File VARCHAR(255) NOT NULL,
  epoch BIGINT UNSIGNED NOT NULL,
  inserts INT UNSIGNED NOT NULL,
  updates INT UNSIGNED NOT NULL,
  deletes INT UNSIGNED NOT NULL,
  schemaops INT UNSIGNED NOT NULL,
  orig_server_id INT UNSIGNED NOT NULL,
  orig_epoch BIGINT UNSIGNED NOT NULL,
  gci INT UNSIGNED NOT NULL,
  next_position BIGINT UNSIGNED NOT NULL,
  next_file VARCHAR(255) NOT NULL,
  PRIMARY KEY(epoch, orig_server_id, orig_epoch)
  ) ENGINE=INNODB CHARACTER SET latin1 STATS_PERSISTENT=0 ROW_FORMAT=DYNAMIC TABLESPACE=mysql";

SET @str = CONCAT(@cmd, " ENCRYPTION='", @is_mysql_encrypted, "'");
SET @str = IF(@have_ndb = 1, @str, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;
