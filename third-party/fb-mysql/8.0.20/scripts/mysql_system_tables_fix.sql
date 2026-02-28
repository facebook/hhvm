-- Copyright (c) 2003, 2020, Oracle and/or its affiliates. All rights reserved.
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

# This part converts any old privilege tables to privilege tables suitable
# for current version of MySQL

# You can safely ignore all 'Duplicate column' and 'Unknown column' errors
# because these just mean that your tables are already up to date.
# This script is safe to run even if your tables are already up to date!

# Warning message(s) produced for a statement can be printed by explicitly
# adding a 'SHOW WARNINGS' after the statement.

set default_storage_engine=InnoDB;

# We meed to turn off the default strict mode in case legacy data contains e.g.
# zero dates ('0000-00-00-00:00:00'), otherwise, we risk to end up with
# e.g. failing ALTER TABLE statements and incorrect table definitions.

SET @old_sql_mode = @@session.sql_mode, @@session.sql_mode = '';

# Create a user mysql.infoschema@localhost as the owner of views in information_schema.
# That user should be created at the beginning of the script, because a query against a
# view from information_schema leads to check for presence of a user specified in view's DEFINER clause.
# If the user mysql.infoschema@localhost hadn't been created at the beginning of the script,
# the query from information_schema.tables below would have failed with the error
# ERROR 1449 (HY000): The user specified as a definer ('mysql.infoschema'@'localhost') does not exist.

INSERT IGNORE INTO mysql.user
(host, user, select_priv, plugin, authentication_string, ssl_cipher, x509_issuer, x509_subject)
VALUES ('localhost','mysql.infoschema','Y','caching_sha2_password','$A$005$THISISACOMBINATIONOFINVALIDSALTANDPASSWORDTHATMUSTNEVERBRBEUSED','','','');

ALTER TABLE user add File_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL;

# Detect whether or not we had the Grant_priv column
SET @hadGrantPriv:=0;
SELECT @hadGrantPriv:=1 FROM user WHERE Grant_priv LIKE '%';

ALTER TABLE user add Grant_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL,add References_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL,add Index_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL,add Alter_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL;
ALTER TABLE db add Grant_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL,add References_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL,add Index_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL,add Alter_priv enum('N','Y') COLLATE utf8_general_ci NOT NULL;

# Fix privileges for old tables
UPDATE user SET Grant_priv=File_priv,References_priv=Create_priv,Index_priv=Create_priv,Alter_priv=Create_priv WHERE @hadGrantPriv = 0;
UPDATE db SET References_priv=Create_priv,Index_priv=Create_priv,Alter_priv=Create_priv WHERE @hadGrantPriv = 0;

#
# The second alter changes ssl_type to new 4.0.2 format
# Adding columns needed by GRANT .. REQUIRE (openssl)

ALTER TABLE user
ADD ssl_type enum('','ANY','X509', 'SPECIFIED') COLLATE utf8_general_ci NOT NULL,
ADD ssl_cipher BLOB NOT NULL,
ADD x509_issuer BLOB NOT NULL,
ADD x509_subject BLOB NOT NULL;
ALTER TABLE user MODIFY ssl_type enum('','ANY','X509', 'SPECIFIED') NOT NULL;

#
# tables_priv
#
ALTER TABLE tables_priv
  ADD KEY Grantor (Grantor);

ALTER TABLE tables_priv
  MODIFY Db char(64) NOT NULL default '',
  MODIFY User char(80) NOT NULL default '',
  MODIFY Table_name char(64) NOT NULL default '',
  CONVERT TO CHARACTER SET utf8 COLLATE utf8_bin;

ALTER TABLE tables_priv
  MODIFY Column_priv set('Select','Insert','Update','References')
    COLLATE utf8_general_ci DEFAULT '' NOT NULL,
  MODIFY Table_priv set('Select','Insert','Update','Delete','Create',
                        'Drop','Grant','References','Index','Alter',
                        'Create View','Show view','Trigger')
    COLLATE utf8_general_ci DEFAULT '' NOT NULL,
  COMMENT='Table privileges';

#
# columns_priv
#
#
# Name change of Type -> Column_priv from MySQL 3.22.12
#
ALTER TABLE columns_priv
  CHANGE Type Column_priv set('Select','Insert','Update','References')
    COLLATE utf8_general_ci DEFAULT '' NOT NULL;

ALTER TABLE columns_priv
  MODIFY Db char(64) NOT NULL default '',
  MODIFY User char(80) NOT NULL default '',
  MODIFY Table_name char(64) NOT NULL default '',
  MODIFY Column_name char(64) NOT NULL default '',
  CONVERT TO CHARACTER SET utf8 COLLATE utf8_bin,
  COMMENT='Column privileges';

ALTER TABLE columns_priv
  MODIFY Column_priv set('Select','Insert','Update','References')
    COLLATE utf8_general_ci DEFAULT '' NOT NULL;

#
#  Add the new 'type' column to the func table.
#

ALTER TABLE func add type enum ('function','aggregate') COLLATE utf8_general_ci NOT NULL;

#
#  Change the user and db tables to current format
#

# Detect whether we had Show_db_priv
SET @hadShowDbPriv:=0;
SELECT @hadShowDbPriv:=1 FROM user WHERE Show_db_priv LIKE '%';

ALTER TABLE user
ADD Show_db_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Alter_priv,
ADD Super_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Show_db_priv,
ADD Create_tmp_table_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Super_priv,
ADD Lock_tables_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_tmp_table_priv,
ADD Execute_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Lock_tables_priv,
ADD Repl_slave_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Execute_priv,
ADD Repl_client_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Repl_slave_priv;

# Convert privileges so that users have similar privileges as before

UPDATE user SET Show_db_priv= Select_priv, Super_priv=Process_priv, Execute_priv=Process_priv, Create_tmp_table_priv='Y', Lock_tables_priv='Y', Repl_slave_priv=file_priv, Repl_client_priv=File_priv where user<>"" AND @hadShowDbPriv = 0;


#  Add fields that can be used to limit number of questions and connections
#  for some users.

ALTER TABLE user
ADD max_questions int NOT NULL DEFAULT 0 AFTER x509_subject,
ADD max_updates   int unsigned NOT NULL DEFAULT 0 AFTER max_questions,
ADD max_connections int unsigned NOT NULL DEFAULT 0 AFTER max_updates;

#
# Update proxies_priv definition.
#
ALTER TABLE proxies_priv MODIFY User char(80) binary DEFAULT '' NOT NULL;
ALTER TABLE proxies_priv MODIFY Proxied_user char(80) binary DEFAULT '' NOT NULL;
ALTER TABLE proxies_priv MODIFY Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL, ENGINE=InnoDB;
ALTER TABLE proxies_priv MODIFY Proxied_host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL;
ALTER TABLE proxies_priv MODIFY Grantor varchar(288) binary DEFAULT '' NOT NULL;

#
#  Add Create_tmp_table_priv and Lock_tables_priv to db
#

ALTER TABLE db
ADD Create_tmp_table_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
ADD Lock_tables_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL;

alter table user change max_questions max_questions int unsigned DEFAULT 0  NOT NULL;


alter table db comment='Database privileges';
alter table user comment='Users and global privileges';
alter table func comment='User defined functions';

# Convert all tables to UTF-8 with binary collation
# and reset all char columns to correct width
ALTER TABLE user
  MODIFY User char(80) NOT NULL default '',
  CONVERT TO CHARACTER SET utf8 COLLATE utf8_bin;
ALTER TABLE user
  MODIFY Select_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Insert_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Update_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Delete_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Create_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Drop_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Reload_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Shutdown_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Process_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY File_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Grant_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY References_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Index_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Alter_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Show_db_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Super_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Create_tmp_table_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Lock_tables_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Execute_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Repl_slave_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY Repl_client_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY ssl_type enum('','ANY','X509', 'SPECIFIED') COLLATE utf8_general_ci DEFAULT '' NOT NULL;

ALTER TABLE db
  MODIFY Db char(64) NOT NULL default '',
  MODIFY User char(80) NOT NULL default '',
  CONVERT TO CHARACTER SET utf8 COLLATE utf8_bin;
ALTER TABLE db
  MODIFY  Select_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Insert_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Update_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Delete_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Create_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Drop_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Grant_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  References_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Index_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Alter_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Create_tmp_table_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL,
  MODIFY  Lock_tables_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL;

ALTER TABLE func CONVERT TO CHARACTER SET utf8 COLLATE utf8_bin;
ALTER TABLE func
  MODIFY type enum ('function','aggregate') COLLATE utf8_general_ci NOT NULL;

#
# Modify log tables.
#

SET @old_log_state = @@global.general_log;
SET GLOBAL general_log = 'OFF';
SET @old_sql_require_primary_key = @@session.sql_require_primary_key;
SET @@session.sql_require_primary_key = 0;
ALTER TABLE general_log
  MODIFY event_time TIMESTAMP(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
  MODIFY user_host MEDIUMTEXT NOT NULL,
  MODIFY thread_id INTEGER NOT NULL,
  MODIFY server_id INTEGER UNSIGNED NOT NULL,
  MODIFY command_type VARCHAR(64) NOT NULL,
  MODIFY argument MEDIUMBLOB NOT NULL;
ALTER TABLE general_log
  MODIFY thread_id BIGINT UNSIGNED NOT NULL;
SET GLOBAL general_log = @old_log_state;

SET @old_log_state = @@global.slow_query_log;
SET GLOBAL slow_query_log = 'OFF';
ALTER TABLE slow_log
  MODIFY start_time TIMESTAMP(6) NOT NULL DEFAULT CURRENT_TIMESTAMP(6) ON UPDATE CURRENT_TIMESTAMP(6),
  MODIFY user_host MEDIUMTEXT NOT NULL,
  MODIFY query_time TIME(6) NOT NULL,
  MODIFY lock_time TIME(6) NOT NULL,
  MODIFY rows_sent INTEGER NOT NULL,
  MODIFY rows_examined INTEGER NOT NULL,
  MODIFY db VARCHAR(512) NOT NULL,
  MODIFY last_insert_id INTEGER NOT NULL,
  MODIFY insert_id INTEGER NOT NULL,
  MODIFY server_id INTEGER UNSIGNED NOT NULL,
  MODIFY sql_text MEDIUMBLOB NOT NULL;
ALTER TABLE slow_log
  ADD COLUMN thread_id INTEGER NOT NULL AFTER sql_text;
ALTER TABLE slow_log
  MODIFY thread_id BIGINT UNSIGNED NOT NULL;
SET GLOBAL slow_query_log = @old_log_state;

SET @@session.sql_require_primary_key = @old_sql_require_primary_key;
ALTER TABLE plugin
  MODIFY name varchar(64) COLLATE utf8_general_ci NOT NULL DEFAULT '',
  MODIFY dl varchar(128) COLLATE utf8_general_ci NOT NULL DEFAULT '',
  CONVERT TO CHARACTER SET utf8 COLLATE utf8_general_ci;

#
# Detect whether we had Create_view_priv
#
SET @hadCreateViewPriv:=0;
SELECT @hadCreateViewPriv:=1 FROM user WHERE Create_view_priv LIKE '%';

#
# Create VIEWs privileges (v5.0)
#
ALTER TABLE db ADD Create_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Lock_tables_priv;
ALTER TABLE db MODIFY Create_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Lock_tables_priv;

ALTER TABLE user ADD Create_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Repl_client_priv;
ALTER TABLE user MODIFY Create_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Repl_client_priv;

#
# Show VIEWs privileges (v5.0)
#
ALTER TABLE db ADD Show_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_view_priv;
ALTER TABLE db MODIFY Show_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_view_priv;

ALTER TABLE user ADD Show_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_view_priv;
ALTER TABLE user MODIFY Show_view_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_view_priv;

#
# Assign create/show view privileges to people who have create provileges
#
UPDATE user SET Create_view_priv=Create_priv, Show_view_priv=Create_priv where user<>"" AND @hadCreateViewPriv = 0;

#
#
#
SET @hadCreateRoutinePriv:=0;
SELECT @hadCreateRoutinePriv:=1 FROM user WHERE Create_routine_priv LIKE '%';

#
# Create PROCEDUREs privileges (v5.0)
#
ALTER TABLE db ADD Create_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Show_view_priv;
ALTER TABLE db MODIFY Create_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Show_view_priv;

ALTER TABLE user ADD Create_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Show_view_priv;
ALTER TABLE user MODIFY Create_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Show_view_priv;

#
# Alter PROCEDUREs privileges (v5.0)
#
ALTER TABLE db ADD Alter_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_routine_priv;
ALTER TABLE db MODIFY Alter_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_routine_priv;

ALTER TABLE user ADD Alter_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_routine_priv;
ALTER TABLE user MODIFY Alter_routine_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_routine_priv;

ALTER TABLE db ADD Execute_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Alter_routine_priv;
ALTER TABLE db MODIFY Execute_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Alter_routine_priv;

#
# Assign create/alter routine privileges to people who have create privileges
#
UPDATE user SET Create_routine_priv=Create_priv, Alter_routine_priv=Alter_priv where user<>"" AND @hadCreateRoutinePriv = 0;
UPDATE db SET Create_routine_priv=Create_priv, Alter_routine_priv=Alter_priv, Execute_priv=Select_priv where user<>"" AND @hadCreateRoutinePriv = 0;

#
# Add max_user_connections resource limit
#
ALTER TABLE user ADD max_user_connections int unsigned DEFAULT '0' NOT NULL AFTER max_connections;

#
# user.Create_user_priv
#

SET @hadCreateUserPriv:=0;
SELECT @hadCreateUserPriv:=1 FROM user WHERE Create_user_priv LIKE '%';

ALTER TABLE user ADD Create_user_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Alter_routine_priv;
ALTER TABLE user MODIFY Create_user_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Alter_routine_priv;
UPDATE user LEFT JOIN db USING (Host,User) SET Create_user_priv='Y'
  WHERE @hadCreateUserPriv = 0 AND
        (user.Grant_priv = 'Y' OR db.Grant_priv = 'Y');

#
# procs_priv
#

ALTER TABLE procs_priv
  MODIFY User char(80) NOT NULL default '',
  CONVERT TO CHARACTER SET utf8 COLLATE utf8_bin;

ALTER TABLE procs_priv
  MODIFY Proc_priv set('Execute','Alter Routine','Grant')
    COLLATE utf8_general_ci DEFAULT '' NOT NULL;

ALTER TABLE procs_priv
  MODIFY Routine_name char(64)
    COLLATE utf8_general_ci DEFAULT '' NOT NULL;

ALTER TABLE procs_priv
  ADD Routine_type enum('FUNCTION','PROCEDURE')
    COLLATE utf8_general_ci NOT NULL AFTER Routine_name;

ALTER TABLE procs_priv
  MODIFY Timestamp timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP AFTER Proc_priv;


#
# EVENT privilege
#
SET @hadEventPriv := 0;
SELECT @hadEventPriv :=1 FROM user WHERE Event_priv LIKE '%';

ALTER TABLE user add Event_priv enum('N','Y') character set utf8 DEFAULT 'N' NOT NULL AFTER Create_user_priv;
ALTER TABLE user MODIFY Event_priv enum('N','Y') character set utf8 DEFAULT 'N' NOT NULL AFTER Create_user_priv;

UPDATE user SET Event_priv=Super_priv WHERE @hadEventPriv = 0;

ALTER TABLE db add Event_priv enum('N','Y') character set utf8 DEFAULT 'N' NOT NULL;
ALTER TABLE db MODIFY Event_priv enum('N','Y') character set utf8 DEFAULT 'N' NOT NULL;

#
# TRIGGER privilege
#

SET @hadTriggerPriv := 0;
SELECT @hadTriggerPriv :=1 FROM user WHERE Trigger_priv LIKE '%';

ALTER TABLE user ADD Trigger_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Event_priv;
ALTER TABLE user MODIFY Trigger_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Event_priv;

ALTER TABLE db ADD Trigger_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL;
ALTER TABLE db MODIFY Trigger_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL;

UPDATE user SET Trigger_priv=Super_priv WHERE @hadTriggerPriv = 0;

#
# user.Create_tablespace_priv
#

SET @hadCreateTablespacePriv := 0;
SELECT @hadCreateTablespacePriv :=1 FROM user WHERE Create_tablespace_priv LIKE '%';

ALTER TABLE user ADD Create_tablespace_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Trigger_priv;
ALTER TABLE user MODIFY Create_tablespace_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Trigger_priv;

UPDATE user SET Create_tablespace_priv = Super_priv WHERE @hadCreateTablespacePriv = 0;

--
-- Unlike 'performance_schema', the 'mysql' database is reserved already,
-- so no user procedure is supposed to be there.
--
-- NOTE: until upgrade is finished, stored routines are not available, because
-- system tables might be not usable.
--
SET @global_automatic_sp_privileges = @@GLOBAL.automatic_sp_privileges;
SET GLOBAL automatic_sp_privileges = FALSE;

drop procedure if exists mysql.die;
create procedure mysql.die() signal sqlstate 'HY000' set message_text='Unexpected content found in the performance_schema database.';

--
-- For broken upgrades, SIGNAL the error
--

SET @cmd="call mysql.die()";

SET @str = IF(@broken_pfs > 0, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

drop procedure mysql.die;
SET GLOBAL automatic_sp_privileges = @global_automatic_sp_privileges;

ALTER TABLE user ADD plugin char(64) DEFAULT 'caching_sha2_password' NOT NULL,  ADD authentication_string TEXT;
ALTER TABLE user MODIFY plugin char(64) DEFAULT 'caching_sha2_password' NOT NULL;
UPDATE user SET plugin=IF((length(password) = 41), 'mysql_native_password', '') WHERE plugin = '';
UPDATE user SET plugin=IF((length(password) = 0), 'caching_sha2_password', '') WHERE plugin = '';
ALTER TABLE user MODIFY authentication_string TEXT;

-- establish if the field is already there.
SET @hadPasswordExpired:=0;
SELECT @hadPasswordExpired:=1 FROM user WHERE password_expired LIKE '%';

ALTER TABLE user ADD password_expired ENUM('N', 'Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL;
UPDATE user SET password_expired = 'N' WHERE @hadPasswordExpired=0;

-- need to compensate for the ALTER TABLE user .. CONVERT TO CHARACTER SET above
ALTER TABLE user MODIFY password_expired ENUM('N', 'Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL;

-- Need to pre-fill mysql.proxies_priv with access for root even when upgrading from
-- older versions
SET @cmd="INSERT INTO proxies_priv VALUES ('localhost', 'root', '', '', TRUE, '', now())";
SET @str = IF(@had_proxies_priv_table = 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

-- Checking for any duplicate hostname and username combination are exists.
-- If exits we will throw error.

-- We also need to avoid accessing privilege tables.
SET @global_automatic_sp_privileges = @@GLOBAL.automatic_sp_privileges;
SET GLOBAL automatic_sp_privileges = FALSE;

DROP PROCEDURE IF EXISTS mysql.warn_duplicate_host_names;
CREATE PROCEDURE mysql.warn_duplicate_host_names() SIGNAL SQLSTATE '45000'  SET MESSAGE_TEXT = 'Multiple accounts exist for @user_name, @host_name that differ only in Host lettercase; remove all except one of them';
SET @cmd='call mysql.warn_duplicate_host_names()';
SET @duplicate_hosts=(SELECT count(*) FROM mysql.user GROUP BY user, lower(host) HAVING count(*) > 1 LIMIT 1);
SET @str=IF(@duplicate_hosts > 1, @cmd, 'SET @dummy=0');

PREPARE stmt FROM @str;
EXECUTE stmt;
-- Get warnings (if any)
SHOW WARNINGS;
DROP PREPARE stmt;
DROP PROCEDURE mysql.warn_duplicate_host_names;

SET GLOBAL automatic_sp_privileges = @global_automatic_sp_privileges;

# Convering the host name to lower case for existing users
UPDATE user SET host=LOWER( host ) WHERE LOWER( host ) <> host;

#
# Alter mysql.component only if it exists already.
#

SET @have_component= (select count(*) from information_schema.tables where table_schema='mysql' and table_name='component');

# Change row format to DYNAMIC
SET @cmd="ALTER TABLE component ROW_FORMAT=DYNAMIC";

SET @str = IF(@have_component = 1, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

#
# Alter mysql.ndb_binlog_index only if it exists already.
#

SET @have_ndb_binlog_index= (select count(*) from information_schema.tables where table_schema='mysql' and table_name='ndb_binlog_index');

# Change type from BIGINT to INT and row format to DYNAMIC
SET @cmd="ALTER TABLE ndb_binlog_index
  MODIFY inserts INT UNSIGNED NOT NULL,
  MODIFY updates INT UNSIGNED NOT NULL,
  MODIFY deletes INT UNSIGNED NOT NULL,
  MODIFY schemaops INT UNSIGNED NOT NULL, ROW_FORMAT=DYNAMIC";

SET @str = IF(@have_ndb_binlog_index = 1, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

# Add new columns
SET @cmd="ALTER TABLE ndb_binlog_index
  ADD orig_server_id INT UNSIGNED NOT NULL,
  ADD orig_epoch BIGINT UNSIGNED NOT NULL,
  ADD gci INT UNSIGNED NOT NULL";

SET @str = IF(@have_ndb_binlog_index = 1, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

# New primary key
SET @cmd="ALTER TABLE ndb_binlog_index
  DROP PRIMARY KEY,
  ADD PRIMARY KEY(epoch, orig_server_id, orig_epoch)";

SET @str = IF(@have_ndb_binlog_index = 1, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

--
-- Check for accounts with old pre-4.1 passwords and issue a warning
--

-- SCRAMBLED_PASSWORD_CHAR_LENGTH_323 = 16
SET @deprecated_pwds=(SELECT COUNT(*) FROM mysql.user WHERE LENGTH(password) = 16);

-- signal the deprecation error

SET @global_automatic_sp_privileges = @@GLOBAL.automatic_sp_privileges;
SET GLOBAL automatic_sp_privileges = FALSE;

DROP PROCEDURE IF EXISTS mysql.warn_pre41_pwd;
CREATE PROCEDURE mysql.warn_pre41_pwd() SIGNAL SQLSTATE '01000' SET MESSAGE_TEXT='Pre-4.1 password hash found. It is deprecated and will be removed in a future release. Please upgrade it to a new format.';
SET @cmd='call mysql.warn_pre41_pwd()';
SET @str=IF(@deprecated_pwds > 0, @cmd, 'SET @dummy=0');
PREPARE stmt FROM @str;
EXECUTE stmt;
-- Get warnings (if any)
SHOW WARNINGS;
DROP PREPARE stmt;
DROP PROCEDURE mysql.warn_pre41_pwd;

SET GLOBAL automatic_sp_privileges = @global_automatic_sp_privileges;
--
-- Add timestamp and expiry columns
--

ALTER TABLE user ADD password_last_changed timestamp NULL;
UPDATE user SET password_last_changed = CURRENT_TIMESTAMP WHERE plugin in ('caching_sha2_password','mysql_native_password','sha256_password') and password_last_changed is NULL;

ALTER TABLE user ADD password_lifetime smallint unsigned NULL;

--
-- Add account_locked column
--
SET @hadAccountLocked:=0;
SELECT @hadAccountLocked:=1 FROM user WHERE account_locked LIKE '%';

ALTER TABLE user ADD account_locked ENUM('N', 'Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL;
UPDATE user SET account_locked = 'N' WHERE @hadAccountLocked=0;

-- need to compensate for the ALTER TABLE user .. CONVERT TO CHARACTER SET above
ALTER TABLE user MODIFY account_locked ENUM('N', 'Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL;

UPDATE user SET account_locked ='Y' WHERE host = 'localhost' AND user = 'mysql.infoschema';

--
-- Drop password column
--

SET @have_password= (SELECT COUNT(*) FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = 'mysql'
                    AND TABLE_NAME='user'
                    AND column_name='password');
SET @str=IF(@have_password <> 0, "UPDATE user SET authentication_string = password where LENGTH(password) > 0 and plugin = 'mysql_native_password'", "SET @dummy = 0");
# We have already put mysql_native_password as plugin value in cases where length(PASSWORD) is either 0 or 41.
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;
SET @str=IF(@have_password <> 0, "ALTER TABLE user DROP password", "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

-- Add the privilege XA_RECOVER_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige XA_RECOVER_ADMIN.
SET @hadXARecoverAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'XA_RECOVER_ADMIN');
INSERT INTO global_grants SELECT user, host, 'XA_RECOVER_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadXARecoverAdminPriv = 0;
COMMIT;

-- Add the privilege CLONE_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige CLONE_ADMIN.
SET @hadCloneAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'CLONE_ADMIN');
INSERT INTO global_grants SELECT user, host, 'CLONE_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadCloneAdminPriv = 0;
COMMIT;

-- Add the privilege BACKUP_ADMIN for every user who has the privilege RELOAD
-- provided that there isn't a user who already has the privilege BACKUP_ADMIN.
SET @hadBackupAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'BACKUP_ADMIN');
INSERT INTO global_grants SELECT user, host, 'BACKUP_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE Reload_priv = 'Y' AND @hadBackupAdminPriv = 0;
COMMIT;

-- Add the privilege INNODB_REDO_LOG_ARCHIVE for every user who has the privilege BACKUP_ADMIN
-- provided that there isn't a user who already has the privilege INNODB_REDO_LOG_ARCHIVE.
SET @hadInnodbRedoLogArchivePriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'INNODB_REDO_LOG_ARCHIVE');
INSERT INTO global_grants SELECT user, host, 'INNODB_REDO_LOG_ARCHIVE', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE Reload_priv = 'Y' AND @hadInnodbRedoLogArchivePriv = 0;
COMMIT;

-- Add the privilege RESOURCE_GROUP_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilege RESOURCE_GROUP_ADMIN.
SET @hadResourceGroupAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'RESOURCE_GROUP_ADMIN');
INSERT INTO global_grants SELECT user, host, 'RESOURCE_GROUP_ADMIN',
IF(grant_priv = 'Y', 'Y', 'N') FROM mysql.user WHERE super_priv = 'Y' AND @hadResourceGroupAdminPriv = 0;
COMMIT;

-- Add the privilege SERVICE_CONNECTION_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilege SERVICE_CONNECTION_ADMIN.
SET @hadServiceConnectionAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'SERVICE_CONNECTION_ADMIN');
INSERT INTO global_grants SELECT user, host, 'SERVICE_CONNECTION_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadServiceConnectionAdminPriv = 0;

-- Add the privilege APPLICATION_PASSWORD_ADMIN for every user who has the
-- privilege CREATE USER provided that there isn't a user who already has
-- privilege APPLICATION_PASSWORD_ADMIN
SET @hadApplicationPasswordAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'APPLICATION_PASSWORD_ADMIN');
INSERT INTO global_grants SELECT user, host, 'APPLICATION_PASSWORD_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE Create_user_priv = 'Y' AND @hadApplicationPasswordAdminPriv = 0;
COMMIT;

-- Add the privilege AUDIT_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige AUDIT_ADMIN.
SET @hadAuditAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'AUDIT_ADMIN');
INSERT INTO global_grants SELECT user, host, 'AUDIT_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadAuditAdminPriv = 0;
COMMIT;

-- Add the privilege BINLOG_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige BINLOG_ADMIN.
SET @hadBinLogAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'BINLOG_ADMIN');
INSERT INTO global_grants SELECT user, host, 'BINLOG_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadBinLogAdminPriv = 0;
COMMIT;

-- Add the privilege BINLOG_ENCRYPTION_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige BINLOG_ENCRYPTION_ADMIN.
SET @hadBinLogEncryptionAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'BINLOG_ENCRYPTION_ADMIN');
INSERT INTO global_grants SELECT user, host, 'BINLOG_ENCRYPTION_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadBinLogEncryptionAdminPriv = 0;
COMMIT;

-- Add the privilege CONNECTION_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige CONNECTION_ADMIN.
SET @hadConnectionAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'CONNECTION_ADMIN');
INSERT INTO global_grants SELECT user, host, 'CONNECTION_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadConnectionAdminPriv = 0;
COMMIT;

-- Add the privilege ENCRYPTION_KEY_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige ENCRYPTION_KEY_ADMIN.
SET @hadEncryptionKeyAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'ENCRYPTION_KEY_ADMIN');
INSERT INTO global_grants SELECT user, host, 'ENCRYPTION_KEY_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadEncryptionKeyAdminPriv = 0;
COMMIT;

-- Add the privilege GROUP_REPLICATION_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige GROUP_REPLICATION_ADMIN.
SET @hadGroupReplicationAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'GROUP_REPLICATION_ADMIN');
INSERT INTO global_grants SELECT user, host, 'GROUP_REPLICATION_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadGroupReplicationAdminPriv = 0;
COMMIT;

-- Add the privilege PERSIST_RO_VARIABLES_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige PERSIST_RO_VARIABLES_ADMIN.
SET @hadPersistRoVariablesAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'PERSIST_RO_VARIABLES_ADMIN');
INSERT INTO global_grants SELECT user, host, 'PERSIST_RO_VARIABLES_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadPersistRoVariablesAdminPriv = 0;
COMMIT;

-- Add the privilege REPLICATION_SLAVE_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige REPLICATION_SLAVE_ADMIN.
SET @hadReplicationSlaveAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'REPLICATION_SLAVE_ADMIN');
INSERT INTO global_grants SELECT user, host, 'REPLICATION_SLAVE_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadReplicationSlaveAdminPriv = 0;
COMMIT;

-- Add the privilege RESOURCE_GROUP_USER for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige RESOURCE_GROUP_USER.
SET @hadResourceGroupUserPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'RESOURCE_GROUP_USER');
INSERT INTO global_grants SELECT user, host, 'RESOURCE_GROUP_USER', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadResourceGroupUserPriv = 0;
COMMIT;

-- Add the privilege ROLE_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige ROLE_ADMIN.
SET @hadRoleAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'ROLE_ADMIN');
INSERT INTO global_grants SELECT user, host, 'ROLE_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadRoleAdminPriv = 0;
COMMIT;

-- Add the privilege SESSION_VARIABLES_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige SESSION_VARIABLES_ADMIN.
SET @hadSessionVariablesAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'SESSION_VARIABLES_ADMIN');
INSERT INTO global_grants SELECT user, host, 'SESSION_VARIABLES_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadSessionVariablesAdminPriv = 0;
COMMIT;

-- Add the privilege SET_USER_ID for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige SET_USER_ID.
SET @hadSetUserIdPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'SET_USER_ID');
INSERT INTO global_grants SELECT user, host, 'SET_USER_ID', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadSetUserIdPriv = 0;
COMMIT;

-- Add the privilege SYSTEM_VARIABLES_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige SYSTEM_VARIABLES_ADMIN.
SET @hadSystemVariablesAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'SYSTEM_VARIABLES_ADMIN');
INSERT INTO global_grants SELECT user, host, 'SYSTEM_VARIABLES_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadSystemVariablesAdminPriv = 0;
COMMIT;

-- Add the privilege SYSTEM_USER for every user who has privilege SET_USER_ID privilege
SET @hadSystemUserPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'SYSTEM_USER');
INSERT INTO global_grants SELECT user, host, 'SYSTEM_USER',
IF (WITH_GRANT_OPTION = 'Y', 'Y', 'N') FROM global_grants WHERE priv = 'SET_USER_ID' AND @hadSystemUserPriv = 0;
COMMIT;

-- Add the privilege SYSTEM_USER for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilege SYSTEM_USER
SET @hadSystemUserPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'SYSTEM_USER');
INSERT INTO global_grants SELECT user, host, 'SYSTEM_USER', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadSystemUserPriv = 0;
COMMIT;

-- Add the privilege TABLE_ENCRYPTION_ADMIN for every user who has the privilege SUPER
-- provided that there isn't a user who already has the privilige TABLE_ENCRYPTION_ADMIN.
SET @hadTableEncryptionAdminPriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'TABLE_ENCRYPTION_ADMIN');
INSERT INTO global_grants SELECT user, host, 'TABLE_ENCRYPTION_ADMIN', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE super_priv = 'Y' AND @hadTableEncryptionAdminPriv = 0 AND user != 'mysql.session';
-- The TABLE_ENCRYPTION_ADMIN privilege was previously granted to 'mysql.session'
-- during upgrade. However, this user should not have this privilege, so we need
-- to explicitly revoke it.
DELETE FROM global_grants WHERE user = 'mysql.session' AND host = 'localhost' AND priv = 'TABLE_ENCRYPTION_ADMIN';
COMMIT;

-- Add the privilege SHOW_ROUTINE for every user who has global SELECT privilege
-- provided that there isn't a user who already has the privilege SHOW_ROUTINE
SET @hadShowRoutinePriv = (SELECT COUNT(*) FROM global_grants WHERE priv = 'SHOW_ROUTINE');
INSERT INTO global_grants SELECT user, host, 'SHOW_ROUTINE', IF(grant_priv = 'Y', 'Y', 'N')
FROM mysql.user WHERE select_priv = 'Y' AND @hadShowRoutinePriv = 0 AND user NOT IN ('mysql.infoschema','mysql.session','mysql.sys');
COMMIT;

# Activate the new, possible modified privilege tables
# This should not be needed, but gives us some extra testing that the above
# changes was correct

ALTER TABLE slave_master_info ADD Ssl_crl TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The file used for the Certificate Revocation List (CRL)';
ALTER TABLE slave_master_info ADD Ssl_crlpath TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The path used for Certificate Revocation List (CRL) files';
ALTER TABLE slave_master_info STATS_PERSISTENT=0;
ALTER TABLE slave_worker_info STATS_PERSISTENT=0;
ALTER TABLE slave_relay_log_info STATS_PERSISTENT=0;
ALTER TABLE gtid_executed STATS_PERSISTENT=0;

#
# From 5.7 onwards, all slave info tables have Channel_Name as a column.
# This column is needed  for multi-source replication
#
ALTER TABLE slave_master_info
  ADD Channel_name CHAR(64) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT 'The channel on which the slave is connected to a source. Used in Multisource Replication',
  DROP PRIMARY KEY,
  ADD PRIMARY KEY(Channel_name);

ALTER TABLE slave_relay_log_info
  ADD Channel_name CHAR(64) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT 'The channel on which the slave is connected to a source. Used in Multisource Replication',
  DROP PRIMARY KEY,
  ADD PRIMARY KEY(Channel_name);

ALTER TABLE slave_worker_info
  ADD Channel_name CHAR(64) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT 'The channel on which the slave is connected to a source. Used in Multisource Replication',
  DROP PRIMARY KEY,
  ADD PRIMARY KEY(Channel_name, Id);

# The Tls_version field at slave_master_info should be added after the Channel_name field
ALTER TABLE slave_master_info ADD Tls_version TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'Tls version';

# If the order of columns Channel_name and Tls_version is wrong, this will correct the order
# in slave_master_info table.
ALTER TABLE slave_master_info
  MODIFY COLUMN Tls_version TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'Tls version'
  AFTER Channel_name;

# The Public_key_path field at slave_master_info should be added after the Tls_version field
ALTER TABLE slave_master_info ADD Public_key_path TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The file containing public key of master server.';

# The Get_public_key field at slave_master_info should be added after the slave_master_info field
ALTER TABLE slave_master_info ADD Get_public_key BOOLEAN NOT NULL COMMENT 'Preference to get public key from master.';

ALTER TABLE slave_master_info ADD Network_namespace TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'Network namespace used for communication with the master server.';

ALTER TABLE slave_master_info ADD Master_compression_algorithm CHAR(64) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL COMMENT 'Compression algorithm supported for data transfer between master and slave.',
                              ADD Master_zstd_compression_level INTEGER UNSIGNED NOT NULL COMMENT 'Compression level associated with zstd compression algorithm.';

ALTER TABLE slave_master_info ADD Tls_ciphersuites TEXT CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL COMMENT 'Ciphersuites used for TLS 1.3 communication with the master server.';

# If the order of column Public_key_path, Get_public_key is wrong, this will correct the order in
# slave_master_info table.
ALTER TABLE slave_master_info
  MODIFY COLUMN Public_key_path TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The file containing public key of master server.'
  AFTER Tls_version;
ALTER TABLE slave_master_info
  MODIFY COLUMN Get_public_key BOOLEAN NOT NULL COMMENT 'Preference to get public key from master.'
  AFTER Public_key_path;

ALTER TABLE slave_master_info
  MODIFY COLUMN Network_namespace TEXT CHARACTER SET utf8 COLLATE utf8_bin
  COMMENT 'Network namespace used for communication with the master server.'
  AFTER Get_public_key;

ALTER TABLE slave_master_info
  MODIFY COLUMN Tls_ciphersuites TEXT CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL
  COMMENT 'Ciphersuites used for TLS 1.3 communication with the master server.'
  AFTER Master_zstd_compression_level;

# Columns added to keep information about the replication applier thread
# privilege context user
ALTER TABLE slave_relay_log_info ADD Privilege_checks_username CHAR(32) COLLATE utf8_bin DEFAULT NULL COMMENT 'Username part of PRIVILEGE_CHECKS_USER.' AFTER Channel_name,
                                 ADD Privilege_checks_hostname CHAR(255) CHARACTER SET ascii COLLATE ascii_general_ci DEFAULT NULL COMMENT 'Hostname part of PRIVILEGE_CHECKS_USER.' AFTER Privilege_checks_username;

# Columns added to keep information about REQUIRE_ROW_FORMAT replication field
ALTER TABLE slave_relay_log_info ADD Require_row_format BOOLEAN DEFAULT 0 COMMENT 'Indicates whether the channel shall only accept row based events.' AFTER Privilege_checks_hostname;

ALTER TABLE slave_relay_log_info MODIFY Relay_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The name of the current relay log file.',
                                 MODIFY Relay_log_pos BIGINT UNSIGNED COMMENT 'The relay log position of the last executed event.',
                                 MODIFY Master_log_name TEXT CHARACTER SET utf8 COLLATE utf8_bin COMMENT 'The name of the master binary log file from which the events in the relay log file were read.',
                                 MODIFY Master_log_pos BIGINT UNSIGNED COMMENT 'The master log position of the last executed event.',
                                 MODIFY Sql_delay INTEGER COMMENT 'The number of seconds that the slave must lag behind the master.',
                                 MODIFY Number_of_workers INTEGER UNSIGNED,
                                 MODIFY Id INTEGER UNSIGNED COMMENT 'Internal Id that uniquely identifies this record.';

# Columns added to keep information about REQUIRE_TABLE_PRIMARY_KEY_CHECK replication field
ALTER TABLE slave_relay_log_info ADD Require_table_primary_key_check ENUM('STREAM','ON','OFF') NOT NULL DEFAULT 'STREAM' COMMENT 'Indicates what is the channel policy regarding tables having primary keys on create and alter table queries' AFTER Require_row_format;

#
# Drop legacy NDB distributed privileges function & procedures
#
DROP function  IF EXISTS mysql.mysql_cluster_privileges_are_distributed;
DROP procedure IF EXISTS mysql.mysql_cluster_backup_privileges;
DROP procedure IF EXISTS mysql.mysql_cluster_move_grant_tables;
DROP procedure IF EXISTS mysql.mysql_cluster_restore_local_privileges;
DROP procedure IF EXISTS mysql.mysql_cluster_restore_privileges;
DROP procedure IF EXISTS mysql.mysql_cluster_restore_privileges_from_local;
DROP procedure IF EXISTS mysql.mysql_cluster_move_privileges;

#
# Alter mysql.ndb_binlog_index only if it exists already.
#
SET @cmd="ALTER TABLE ndb_binlog_index
  ADD COLUMN next_position BIGINT UNSIGNED NOT NULL";

SET @str = IF(@have_ndb_binlog_index = 1, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE ndb_binlog_index
  ADD COLUMN next_file VARCHAR(255) NOT NULL";

SET @str = IF(@have_ndb_binlog_index = 1, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE ndb_binlog_index
  ENGINE=InnoDB STATS_PERSISTENT=0";

SET @str = IF(@have_ndb_binlog_index = 1, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

--
-- Check for non-empty host table and issue a warning
--

SET @global_automatic_sp_privileges = @@GLOBAL.automatic_sp_privileges;
SET GLOBAL automatic_sp_privileges = FALSE;

DROP PROCEDURE IF EXISTS mysql.warn_host_table_nonempty;
CREATE PROCEDURE mysql.warn_host_table_nonempty() SIGNAL SQLSTATE '01000' SET MESSAGE_TEXT='Table mysql.host is not empty. It is deprecated and will be removed in a future release.';
SET @cmd='call mysql.warn_host_table_nonempty()';

SET @have_host_table=0;
SET @host_table_nonempty=0;
SET @have_host_table=(SELECT COUNT(*) FROM information_schema.tables WHERE table_name LIKE 'host' AND table_schema LIKE 'mysql' AND table_type LIKE 'BASE TABLE');

SET @host_table_nonempty_str=IF(@have_host_table > 0, 'SET @host_table_nonempty=(SELECT COUNT(*) FROM mysql.host)', 'SET @dummy=0');
PREPARE stmt FROM @host_table_nonempty_str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @str=IF(@host_table_nonempty > 0, @cmd, 'SET @dummy=0');

PREPARE stmt FROM @str;
EXECUTE stmt;
-- Get warnings (if any)
SHOW WARNINGS;
DROP PREPARE stmt;
DROP PROCEDURE mysql.warn_host_table_nonempty;

SET GLOBAL automatic_sp_privileges = @global_automatic_sp_privileges;
--
-- Upgrade help tables
--

ALTER TABLE help_category MODIFY url TEXT NOT NULL;
ALTER TABLE help_topic MODIFY url TEXT NOT NULL;

--
-- Upgrade a table engine from MyISAM to InnoDB for the system tables
-- help_topic, help_category, help_relation, help_keyword, plugin, servers,
-- time_zone, time_zone_leap_second, time_zone_name, time_zone_transition,
-- time_zone_transition_type, columns_priv, db, procs_priv, proxies_priv,
-- tables_priv, user.
ALTER TABLE func ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE help_topic ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE help_category ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE help_relation ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE help_keyword ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE plugin ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE servers ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE time_zone ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE time_zone_leap_second ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE time_zone_name ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE time_zone_transition ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE time_zone_transition_type ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE db ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE user ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE tables_priv ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE columns_priv ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE procs_priv ENGINE=InnoDB STATS_PERSISTENT=0;
ALTER TABLE proxies_priv ENGINE=InnoDB STATS_PERSISTENT=0;

--
-- CREATE_ROLE_ACL and DROP_ROLE_ACL
--
ALTER TABLE user ADD Create_role_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER account_locked;
ALTER TABLE user MODIFY Create_role_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER account_locked;
ALTER TABLE user ADD Drop_role_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_role_priv;
ALTER TABLE user MODIFY Drop_role_priv enum('N','Y') COLLATE utf8_general_ci DEFAULT 'N' NOT NULL AFTER Create_role_priv;
UPDATE user SET Create_role_priv= 'Y', Drop_role_priv= 'Y' WHERE Create_user_priv = 'Y';

--
-- Password_reuse_history, Password_reuse_time and Password_require_current
--
ALTER TABLE user ADD Password_reuse_history smallint unsigned NULL DEFAULT NULL AFTER Drop_role_priv;
ALTER TABLE user ADD Password_reuse_time smallint unsigned NULL DEFAULT NULL AFTER Password_reuse_history;
ALTER TABLE user ADD Password_require_current enum('N', 'Y') COLLATE utf8_general_ci DEFAULT NULL AFTER Password_reuse_time;
ALTER TABLE user MODIFY Password_require_current enum('N','Y') COLLATE utf8_general_ci DEFAULT NULL AFTER Password_reuse_time;
ALTER TABLE user ADD User_attributes JSON DEFAULT NULL AFTER Password_require_current;

--
-- Change engine of the firewall tables to InnoDB
--
SET @had_firewall_whitelist =
  (SELECT COUNT(table_name) FROM information_schema.tables
     WHERE table_schema = 'mysql' AND table_name = 'firewall_whitelist' AND
           table_type = 'BASE TABLE');
SET @cmd="ALTER TABLE mysql.firewall_whitelist ENGINE=InnoDB";
SET @str = IF(@had_firewall_whitelist > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;
SET @cmd="ALTER TABLE mysql.firewall_whitelist "
         "MODIFY COLUMN USERHOST VARCHAR(288) NOT NULL";
SET @str = IF(@had_firewall_whitelist > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @had_firewall_users =
  (SELECT COUNT(table_name) FROM information_schema.tables
     WHERE table_schema = 'mysql' AND table_name = 'firewall_users' AND
           table_type = 'BASE TABLE');
SET @cmd="ALTER TABLE mysql.firewall_users ENGINE=InnoDB";
SET @str = IF(@had_firewall_users > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;
SET @cmd="ALTER TABLE mysql.firewall_users "
         "MODIFY COLUMN USERHOST VARCHAR(288)";
SET @str = IF(@had_firewall_users > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

--
-- Add a column that serves as a primary key of the table
--
SET @firewall_whitelist_id_column =
  (SELECT COUNT(column_name) FROM information_schema.columns
     WHERE table_schema = 'mysql' AND table_name = 'firewall_whitelist' AND column_name = 'ID');
SET @cmd="ALTER TABLE mysql.firewall_whitelist ADD ID INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY";
SET @str = IF(@had_firewall_whitelist > 0 AND @firewall_whitelist_id_column = 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

--
-- Change engine, size and charset for audit log tables.
--

SET @had_audit_log_user =
  (SELECT COUNT(table_name) FROM information_schema.tables
     WHERE table_schema = 'mysql' AND table_name = 'audit_log_user' AND
           table_type = 'BASE TABLE');
SET @cmd="ALTER TABLE mysql.audit_log_user DROP FOREIGN KEY audit_log_user_ibfk_1";
SET @str = IF(@had_audit_log_user > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;
SET @cmd="ALTER TABLE mysql.audit_log_user MODIFY COLUMN HOST VARCHAR(255) BINARY NOT NULL";
SET @str = IF(@had_audit_log_user > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @had_audit_log_filter =
  (SELECT COUNT(table_name) FROM information_schema.tables
     WHERE table_schema = 'mysql' AND table_name = 'audit_log_filter' AND
           table_type = 'BASE TABLE');
SET @cmd="ALTER TABLE mysql.audit_log_filter ENGINE=InnoDB";
SET @str = IF(@had_audit_log_filter > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE mysql.audit_log_filter CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci";
SET @str = IF(@had_audit_log_filter > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @had_audit_log_user =
  (SELECT COUNT(table_name) FROM information_schema.tables
     WHERE table_schema = 'mysql' AND table_name = 'audit_log_user' AND
           table_type = 'BASE TABLE');
SET @cmd="ALTER TABLE mysql.audit_log_user ENGINE=InnoDB";
SET @str = IF(@had_audit_log_user > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE mysql.audit_log_user CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci";
SET @str = IF(@had_audit_log_user > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE mysql.audit_log_user MODIFY COLUMN USER VARCHAR(32)";
SET @str = IF(@had_audit_log_user > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE mysql.audit_log_user ADD FOREIGN KEY (FILTERNAME) REFERENCES mysql.audit_log_filter(NAME)";
SET @str = IF(@had_audit_log_user > 0, @cmd, "SET @dummy = 0");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

--
-- Update default_value column for cost tables to new defaults
-- Note: Column definition must be updated if a default value is changed
-- (Must check if column exists to determine whether to add or modify column)
--

-- Update column definition for mysql.server_cost.default_value
SET @have_server_cost_default =
  (SELECT COUNT(column_name) FROM information_schema.columns
     WHERE table_schema = 'mysql' AND table_name = 'server_cost' AND
           column_name = 'default_value');
SET @op = IF(@have_server_cost_default > 0, "MODIFY COLUMN ", "ADD COLUMN ");
SET @str = CONCAT("ALTER TABLE mysql.server_cost ", @op,
   "default_value FLOAT GENERATED ALWAYS AS
    (CASE cost_name
       WHEN 'disk_temptable_create_cost' THEN 20.0
       WHEN 'disk_temptable_row_cost' THEN 0.5
       WHEN 'key_compare_cost' THEN 0.05
       WHEN 'memory_temptable_create_cost' THEN 1.0
       WHEN 'memory_temptable_row_cost' THEN 0.1
       WHEN 'row_evaluate_cost' THEN 0.1
       ELSE NULL
     END) VIRTUAL");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

-- Update column definition for mysql.engine_cost.default_value
SET @have_engine_cost_default =
  (SELECT COUNT(column_name) FROM information_schema.columns
     WHERE table_schema = 'mysql' AND table_name = 'engine_cost' AND
           column_name = 'default_value');
SET @op = IF(@have_engine_cost_default > 0, "MODIFY COLUMN ", "ADD COLUMN ");
SET @str = CONCAT("ALTER TABLE mysql.engine_cost ", @op,
   "default_value FLOAT GENERATED ALWAYS AS
    (CASE cost_name
       WHEN 'io_block_read_cost' THEN 1.0
       WHEN 'memory_block_read_cost' THEN 0.25
       ELSE NULL
     END) VIRTUAL");
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

#
# SQL commands for creating the user in MySQL Server which can be used by the
# internal server session service
# Notes:
# This user is disabled for login
# This user has:
# Select privileges into performance schema tables the mysql.user table.
# SUPER, PERSIST_RO_VARIABLES_ADMIN, SYSTEM_VARIABLES_ADMIN, BACKUP_ADMIN,
# CLONE_ADMIN, SHUTDOWN privileges
#

INSERT IGNORE INTO mysql.user VALUES ('localhost','mysql.session','N','N','N','N','N','N','N','Y','N','N','N','N','N','N','N','Y','N','N','N','N','N','N','N','N','N','N','N','N','N','','','','',0,0,0,0,'caching_sha2_password','$A$005$THISISACOMBINATIONOFINVALIDSALTANDPASSWORDTHATMUSTNEVERBRBEUSED','N',CURRENT_TIMESTAMP,NULL,'Y', 'N', 'N', NULL, NULL, NULL, NULL);

UPDATE user SET Create_role_priv= 'N', Drop_role_priv= 'N' WHERE User= 'mysql.session';
UPDATE user SET Shutdown_priv= 'Y' WHERE User= 'mysql.session';

INSERT IGNORE INTO mysql.tables_priv VALUES ('localhost', 'mysql', 'mysql.session', 'user', 'root\@localhost', CURRENT_TIMESTAMP, 'Select', '');

INSERT IGNORE INTO mysql.db VALUES ('localhost', 'performance_schema', 'mysql.session','Y','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N');

INSERT IGNORE INTO mysql.global_grants VALUES ('mysql.session', 'localhost', 'PERSIST_RO_VARIABLES_ADMIN', 'N');

INSERT IGNORE INTO mysql.global_grants VALUES ('mysql.session', 'localhost', 'SYSTEM_VARIABLES_ADMIN', 'N');

INSERT IGNORE INTO mysql.global_grants VALUES ('mysql.session', 'localhost', 'SESSION_VARIABLES_ADMIN', 'N');

INSERT IGNORE INTO mysql.global_grants VALUES ('mysql.session', 'localhost', 'BACKUP_ADMIN', 'N');

INSERT IGNORE INTO mysql.global_grants VALUES ('mysql.session', 'localhost', 'CLONE_ADMIN', 'N');

INSERT IGNORE INTO mysql.global_grants VALUES ('mysql.session', 'localhost', 'CONNECTION_ADMIN', 'N');

# mysql.session is granted the SUPER and other administrative privileges.
# This user should not be modified inadvertently. Therefore, server grants
# the SYSTEM_USER privilege to this user at the time of initialization or
# upgrade.
INSERT IGNORE INTO mysql.global_grants VALUES ('mysql.session', 'localhost', 'SYSTEM_USER', 'N');

# Move all system tables with InnoDB storage engine to mysql tablespace.
SET @cmd="ALTER TABLE mysql.db TABLESPACE = mysql";
PREPARE stmt FROM @cmd;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE mysql.user TABLESPACE = mysql";
PREPARE stmt FROM @cmd;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE mysql.tables_priv TABLESPACE = mysql";
PREPARE stmt FROM @cmd;
EXECUTE stmt;
DROP PREPARE stmt;


SET @cmd="ALTER TABLE mysql.columns_priv TABLESPACE = mysql";
PREPARE stmt FROM @cmd;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE mysql.procs_priv TABLESPACE = mysql";
PREPARE stmt FROM @cmd;
EXECUTE stmt;
DROP PREPARE stmt;

SET @cmd="ALTER TABLE mysql.proxies_priv TABLESPACE = mysql";
PREPARE stmt FROM @cmd;
EXECUTE stmt;
DROP PREPARE stmt;

# Alter mysql.ndb_binlog_index only if it exists already.
SET @cmd="ALTER TABLE ndb_binlog_index TABLESPACE = mysql";
SET @str = IF(@have_ndb_binlog_index = 1, @cmd, 'SET @dummy = 0');
PREPARE stmt FROM @str;
EXECUTE stmt;
DROP PREPARE stmt;

ALTER TABLE mysql.func TABLESPACE = mysql;
ALTER TABLE mysql.plugin TABLESPACE = mysql;
ALTER TABLE mysql.servers TABLESPACE = mysql;
ALTER TABLE mysql.help_topic TABLESPACE = mysql;
ALTER TABLE mysql.help_category TABLESPACE = mysql;
ALTER TABLE mysql.help_relation TABLESPACE = mysql;
ALTER TABLE mysql.help_keyword TABLESPACE = mysql;
ALTER TABLE mysql.time_zone_name TABLESPACE = mysql;
ALTER TABLE mysql.time_zone TABLESPACE = mysql;
ALTER TABLE mysql.time_zone_transition TABLESPACE = mysql;
ALTER TABLE mysql.time_zone_transition_type TABLESPACE = mysql;
ALTER TABLE mysql.time_zone_leap_second TABLESPACE = mysql;
ALTER TABLE mysql.slave_relay_log_info TABLESPACE = mysql;
ALTER TABLE mysql.slave_master_info TABLESPACE = mysql;
ALTER TABLE mysql.slave_worker_info TABLESPACE = mysql;
ALTER TABLE mysql.gtid_executed TABLESPACE = mysql;
ALTER TABLE mysql.server_cost TABLESPACE = mysql;
ALTER TABLE mysql.engine_cost TABLESPACE = mysql;


# Increase host name length. We need a separate ALTER TABLE to
# alter the CHARACTER SET to ASCII, because the syntax
# 'CONVERT TO CHARACTER...' above changes all field charset
# to utf8_bin.

ALTER TABLE db
  MODIFY Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL;

ALTER TABLE user
  MODIFY Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL;

ALTER TABLE default_roles
MODIFY HOST CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
MODIFY DEFAULT_ROLE_HOST CHAR(255) CHARACTER SET ASCII DEFAULT '%' NOT NULL;

ALTER TABLE role_edges
MODIFY FROM_HOST CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
MODIFY TO_HOST CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL;

ALTER TABLE global_grants
MODIFY HOST CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL;

ALTER TABLE password_history
MODIFY Host CHAR(255) CHARACTER SET ASCII DEFAULT '' NOT NULL;

ALTER TABLE servers
MODIFY Host char(255) CHARACTER SET ASCII NOT NULL DEFAULT '';

ALTER TABLE tables_priv
MODIFY Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
MODIFY Grantor varchar(288) binary DEFAULT '' NOT NULL;

ALTER TABLE columns_priv
MODIFY Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL;

ALTER TABLE slave_master_info
MODIFY Host CHAR(255) CHARACTER SET ASCII COMMENT 'The host name of the master.';

ALTER TABLE procs_priv
MODIFY Host char(255) CHARACTER SET ASCII DEFAULT '' NOT NULL,
MODIFY Grantor varchar(288) binary DEFAULT '' NOT NULL;

# Increase the length of the user column

ALTER TABLE default_roles
MODIFY USER CHAR(80) BINARY DEFAULT '' NOT NULL,
MODIFY DEFAULT_ROLE_USER char (80) binary DEFAULT '' NOT NULL;

ALTER TABLE role_edges
MODIFY FROM_USER CHAR(80) BINARY DEFAULT '' NOT NULL,
MODIFY TO_USER CHAR(80) BINARY DEFAULT '' NOT NULL;

ALTER TABLE global_grants
MODIFY USER CHAR(80) BINARY DEFAULT '' NOT NULL;

ALTER TABLE password_history
MODIFY User CHAR(80) BINARY DEFAULT '' NOT NULL;

ALTER TABLE columns_priv
MODIFY User char(80) binary DEFAULT '' NOT NULL;

# Update the table row format to DYNAMIC
ALTER TABLE columns_priv ROW_FORMAT=DYNAMIC;
ALTER TABLE db ROW_FORMAT=DYNAMIC;
ALTER TABLE default_roles ROW_FORMAT=DYNAMIC;
ALTER TABLE engine_cost ROW_FORMAT=DYNAMIC;
ALTER TABLE func ROW_FORMAT=DYNAMIC;
ALTER TABLE global_grants ROW_FORMAT=DYNAMIC;
ALTER TABLE gtid_executed ROW_FORMAT=DYNAMIC;
ALTER TABLE help_category ROW_FORMAT=DYNAMIC;
ALTER TABLE help_keyword ROW_FORMAT=DYNAMIC;
ALTER TABLE help_relation ROW_FORMAT=DYNAMIC;
ALTER TABLE help_topic ROW_FORMAT=DYNAMIC;
ALTER TABLE plugin ROW_FORMAT=DYNAMIC;
ALTER TABLE password_history ROW_FORMAT=DYNAMIC;
ALTER TABLE procs_priv ROW_FORMAT=DYNAMIC;
ALTER TABLE proxies_priv ROW_FORMAT=DYNAMIC;
ALTER TABLE role_edges ROW_FORMAT=DYNAMIC;
ALTER TABLE servers ROW_FORMAT=DYNAMIC;
ALTER TABLE server_cost ROW_FORMAT=DYNAMIC;
ALTER TABLE slave_master_info ROW_FORMAT=DYNAMIC;
ALTER TABLE slave_worker_info ROW_FORMAT=DYNAMIC;
ALTER TABLE slave_relay_log_info ROW_FORMAT=DYNAMIC;
ALTER TABLE tables_priv ROW_FORMAT=DYNAMIC;
ALTER TABLE time_zone ROW_FORMAT=DYNAMIC;
ALTER TABLE time_zone_name ROW_FORMAT=DYNAMIC;
ALTER TABLE time_zone_leap_second ROW_FORMAT=DYNAMIC;
ALTER TABLE time_zone_transition ROW_FORMAT=DYNAMIC;
ALTER TABLE time_zone_transition_type ROW_FORMAT=DYNAMIC;
ALTER TABLE user ROW_FORMAT=DYNAMIC;

SET @@session.sql_mode = @old_sql_mode;
