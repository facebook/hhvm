--  Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; version 2 of the License.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

DROP PROCEDURE IF EXISTS execute_prepared_stmt;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE execute_prepared_stmt (
        IN in_query longtext CHARACTER SET UTF8MB4
    )
    COMMENT '
Description
-----------

Takes the query in the argument and executes it using a prepared statement. The prepared statement is deallocated,
so the procedure is mainly useful for executing one off dynamically created queries.

The sys_execute_prepared_stmt prepared statement name is used for the query and is required not to exist.


Parameters
-----------

in_query (longtext CHARACTER SET UTF8MB4):
  The query to execute.


Configuration Options
----------------------

sys.debug
  Whether to provide debugging output.
  Default is ''OFF''. Set to ''ON'' to include.


Example
--------

mysql> CALL sys.execute_prepared_stmt(''SELECT * FROM sys.sys_config'');
+------------------------+-------+---------------------+--------+
| variable               | value | set_time            | set_by |
+------------------------+-------+---------------------+--------+
| statement_truncate_len | 64    | 2015-06-30 13:06:00 | NULL   |
+------------------------+-------+---------------------+--------+
1 row in set (0.00 sec)

Query OK, 0 rows affected (0.00 sec)
'
    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    READS SQL DATA
BEGIN
    -- Set configuration options
    IF (@sys.debug IS NULL) THEN
        SET @sys.debug = sys.sys_get_config('debug', 'OFF');
    END IF;

    -- Verify the query exists
    -- The shortest possible query is "DO 1"
    IF (in_query IS NULL OR LENGTH(in_query) < 4) THEN
       SIGNAL SQLSTATE '45000'
          SET MESSAGE_TEXT = "The @sys.execute_prepared_stmt.sql must contain a query";
    END IF;

    SET @sys.execute_prepared_stmt.sql = in_query;

    IF (@sys.debug = 'ON') THEN
        SELECT @sys.execute_prepared_stmt.sql AS 'Debug';
    END IF;
    PREPARE sys_execute_prepared_stmt FROM @sys.execute_prepared_stmt.sql;
    EXECUTE sys_execute_prepared_stmt;
    DEALLOCATE PREPARE sys_execute_prepared_stmt;

    SET @sys.execute_prepared_stmt.sql = NULL;
END$$

DELIMITER ;
