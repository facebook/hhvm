-- Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
-- 
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; version 2 of the License.
-- 
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- 
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

DROP PROCEDURE IF EXISTS table_exists;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE table_exists (
        IN in_db VARCHAR(64), IN in_table VARCHAR(64),
        OUT out_exists ENUM('', 'BASE TABLE', 'VIEW', 'TEMPORARY')
    )
    COMMENT '
Description
-----------

Tests whether the table specified in in_db and in_table exists either as a regular
table, or as a temporary table. The returned value corresponds to the table that
will be used, so if there''s both a temporary and a permanent table with the given
name, then ''TEMPORARY'' will be returned.

Parameters
-----------

in_db (VARCHAR(64)):
  The database name to check for the existance of the table in.

in_table (VARCHAR(64)):
  The name of the table to check the existance of.

out_exists ENUM('''', ''BASE TABLE'', ''VIEW'', ''TEMPORARY''):
  The return value: whether the table exists. The value is one of:
    * ''''           - the table does not exist neither as a base table, view, nor temporary table.
    * ''BASE TABLE'' - the table name exists as a permanent base table table.
    * ''VIEW''       - the table name exists as a view.
    * ''TEMPORARY''  - the table name exists as a temporary table.

Example
--------

mysql> CREATE DATABASE db1;
Query OK, 1 row affected (0.07 sec)

mysql> use db1;
Database changed
mysql> CREATE TABLE t1 (id INT PRIMARY KEY);
Query OK, 0 rows affected (0.08 sec)

mysql> CREATE TABLE t2 (id INT PRIMARY KEY);
Query OK, 0 rows affected (0.08 sec)

mysql> CREATE view v_t1 AS SELECT * FROM t1;
Query OK, 0 rows affected (0.00 sec)

mysql> CREATE TEMPORARY TABLE t1 (id INT PRIMARY KEY);
Query OK, 0 rows affected (0.00 sec)

mysql> CALL sys.table_exists(''db1'', ''t1'', @exists); SELECT @exists;
Query OK, 0 rows affected (0.00 sec)

+------------+
| @exists    |
+------------+
| TEMPORARY  |
+------------+
1 row in set (0.00 sec)

mysql> CALL sys.table_exists(''db1'', ''t2'', @exists); SELECT @exists;
Query OK, 0 rows affected (0.00 sec)

+------------+
| @exists    |
+------------+
| BASE TABLE |
+------------+
1 row in set (0.01 sec)

mysql> CALL sys.table_exists(''db1'', ''v_t1'', @exists); SELECT @exists;
Query OK, 0 rows affected (0.00 sec)

+---------+
| @exists |
+---------+
| VIEW    |
+---------+
1 row in set (0.00 sec)

mysql> CALL sys.table_exists(''db1'', ''t3'', @exists); SELECT @exists;
Query OK, 0 rows affected (0.01 sec)

+---------+
| @exists |
+---------+
|         |
+---------+
1 row in set (0.00 sec)
'
    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    CONTAINS SQL
BEGIN
    DECLARE v_error BOOLEAN DEFAULT FALSE;
    DECLARE CONTINUE HANDLER FOR 1050 SET v_error = TRUE;
    DECLARE CONTINUE HANDLER FOR 1146 SET v_error = TRUE;

    SET out_exists = '';

    -- Verify whether the table name exists as a normal table
    IF (EXISTS(SELECT 1 FROM information_schema.TABLES WHERE TABLE_SCHEMA = in_db AND TABLE_NAME = in_table)) THEN
        -- Unfortunately the only way to determine whether there is also a temporary table is to try to create
        -- a temporary table with the same name. If it succeeds the table didn't exist as a temporary table.
        SET @sys.tmp.table_exists.SQL = CONCAT('CREATE TEMPORARY TABLE `', in_db, '`.`', in_table, '` (id INT PRIMARY KEY)');
        PREPARE stmt_create_table FROM @sys.tmp.table_exists.SQL;
        EXECUTE stmt_create_table;
        DEALLOCATE PREPARE stmt_create_table;
        IF (v_error) THEN
            SET out_exists = 'TEMPORARY';
        ELSE
            -- The temporary table was created, i.e. it didn't exist. Remove it again so we don't leave garbage around.
            SET @sys.tmp.table_exists.SQL = CONCAT('DROP TEMPORARY TABLE `', in_db, '`.`', in_table, '`');
            PREPARE stmt_drop_table FROM @sys.tmp.table_exists.SQL;
            EXECUTE stmt_drop_table;
            DEALLOCATE PREPARE stmt_drop_table;
            SET out_exists = (SELECT TABLE_TYPE FROM information_schema.TABLES WHERE TABLE_SCHEMA = in_db AND TABLE_NAME = in_table);
        END IF;
    ELSE
        -- Check whether a temporary table exists with the same name.
        -- If it does it's possible to SELECT from the table without causing an error.
        -- If it does not exist even a PREPARE using the table will fail.
        SET @sys.tmp.table_exists.SQL = CONCAT('SELECT COUNT(*) FROM `', in_db, '`.`', in_table, '`');
        PREPARE stmt_select FROM @sys.tmp.table_exists.SQL;
        IF (NOT v_error) THEN
            DEALLOCATE PREPARE stmt_select;
            SET out_exists = 'TEMPORARY';
        END IF;
    END IF;
END$$

DELIMITER ;
