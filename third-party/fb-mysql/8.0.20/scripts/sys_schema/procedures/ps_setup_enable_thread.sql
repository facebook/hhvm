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

DROP PROCEDURE IF EXISTS ps_setup_enable_thread;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE ps_setup_enable_thread (
        IN in_connection_id BIGINT
    )
    COMMENT '
Description
-----------

Enable the given connection/thread in Performance Schema.

Parameters
-----------

in_connection_id (BIGINT):
  The connection ID (PROCESSLIST_ID from performance_schema.threads
  or the ID shown within SHOW PROCESSLIST)

Example
-----------

mysql> CALL sys.ps_setup_enable_thread(3);
+------------------+
| summary          |
+------------------+
| Enabled 1 thread |
+------------------+
1 row in set (0.01 sec)

To enable the current connection:

mysql> CALL sys.ps_setup_enable_thread(CONNECTION_ID());
+------------------+
| summary          |
+------------------+
| Enabled 1 thread |
+------------------+
1 row in set (0.00 sec)
'
    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    MODIFIES SQL DATA
BEGIN
    UPDATE performance_schema.threads
       SET instrumented = 'YES'
     WHERE processlist_id = in_connection_id;

    SELECT CONCAT('Enabled ', @rows := ROW_COUNT(), ' thread', IF(@rows != 1, 's', '')) AS summary;
END$$

DELIMITER ;
