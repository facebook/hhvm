-- Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.
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

DROP PROCEDURE IF EXISTS create_synonym_db;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE create_synonym_db (
        IN in_db_name VARCHAR(64), 
        IN in_synonym VARCHAR(64)
    )
    COMMENT '
Description
-----------

Takes a source database name and synonym name, and then creates the 
synonym database with views that point to all of the tables within
the source database.

Useful for creating a "ps" synonym for "performance_schema",
or "is" instead of "information_schema", for example.

Parameters
-----------

in_db_name (VARCHAR(64)):
  The database name that you would like to create a synonym for.
in_synonym (VARCHAR(64)):
  The database synonym name.

Example
-----------

mysql> SHOW DATABASES;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| sys                |
| test               |
+--------------------+
5 rows in set (0.00 sec)

mysql> CALL sys.create_synonym_db(\'performance_schema\', \'ps\');
+---------------------------------------+
| summary                               |
+---------------------------------------+
| Created 74 views in the `ps` database |
+---------------------------------------+
1 row in set (8.57 sec)

Query OK, 0 rows affected (8.57 sec)

mysql> SHOW DATABASES;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| ps                 |
| sys                |
| test               |
+--------------------+
6 rows in set (0.00 sec)

mysql> SHOW FULL TABLES FROM ps;
+------------------------------------------------------+------------+
| Tables_in_ps                                         | Table_type |
+------------------------------------------------------+------------+
| accounts                                             | VIEW       |
| cond_instances                                       | VIEW       |
| events_stages_current                                | VIEW       |
| events_stages_history                                | VIEW       |
...
'
    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    MODIFIES SQL DATA
BEGIN
    DECLARE v_done bool DEFAULT FALSE;
    DECLARE v_db_name_check VARCHAR(64);
    DECLARE v_db_err_msg TEXT;
    DECLARE v_table VARCHAR(64);
    DECLARE v_views_created INT DEFAULT 0;

    DECLARE db_doesnt_exist CONDITION FOR SQLSTATE '42000';
    DECLARE db_name_exists CONDITION FOR SQLSTATE 'HY000';

    DECLARE c_table_names CURSOR FOR 
        SELECT TABLE_NAME 
          FROM INFORMATION_SCHEMA.TABLES 
         WHERE TABLE_SCHEMA = in_db_name;

    DECLARE CONTINUE HANDLER FOR NOT FOUND SET v_done = TRUE;

    -- Check if the source database exists
    SELECT SCHEMA_NAME INTO v_db_name_check
      FROM INFORMATION_SCHEMA.SCHEMATA
     WHERE SCHEMA_NAME = in_db_name;

    IF v_db_name_check IS NULL THEN
        SET v_db_err_msg = CONCAT('Unknown database ', in_db_name);
        SIGNAL SQLSTATE 'HY000'
            SET MESSAGE_TEXT = v_db_err_msg;
    END IF;

    -- Check if a database of the synonym name already exists
    SELECT SCHEMA_NAME INTO v_db_name_check
      FROM INFORMATION_SCHEMA.SCHEMATA
     WHERE SCHEMA_NAME = in_synonym;

    IF v_db_name_check = in_synonym THEN
        SET v_db_err_msg = CONCAT('Can\'t create database ', in_synonym, '; database exists');
        SIGNAL SQLSTATE 'HY000'
            SET MESSAGE_TEXT = v_db_err_msg;
    END IF;

    -- All good, create the database and views
    SET @create_db_stmt := CONCAT('CREATE DATABASE ', sys.quote_identifier(in_synonym));
    PREPARE create_db_stmt FROM @create_db_stmt;
    EXECUTE create_db_stmt;
    DEALLOCATE PREPARE create_db_stmt;

    SET v_done = FALSE;
    OPEN c_table_names;
    c_table_names: LOOP
        FETCH c_table_names INTO v_table;
        IF v_done THEN
            LEAVE c_table_names;
        END IF;

        SET @create_view_stmt = CONCAT(
            'CREATE SQL SECURITY INVOKER VIEW ',
            sys.quote_identifier(in_synonym),
            '.',
            sys.quote_identifier(v_table),
            ' AS SELECT * FROM ',
            sys.quote_identifier(in_db_name),
            '.',
            sys.quote_identifier(v_table)
        );
        PREPARE create_view_stmt FROM @create_view_stmt;
        EXECUTE create_view_stmt;
        DEALLOCATE PREPARE create_view_stmt;

        SET v_views_created = v_views_created + 1;
    END LOOP;
    CLOSE c_table_names;

    SELECT CONCAT(
        'Created ', v_views_created, ' view',
        IF(v_views_created != 1, 's', ''), ' in the ',
        sys.quote_identifier(in_synonym), ' database'
    ) AS summary;

END$$

DELIMITER ;
