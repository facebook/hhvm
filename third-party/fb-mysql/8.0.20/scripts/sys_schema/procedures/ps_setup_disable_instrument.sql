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

DROP PROCEDURE IF EXISTS ps_setup_disable_instrument;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE ps_setup_disable_instrument (
        IN in_pattern VARCHAR(128)
    )
    COMMENT '
Description
-----------

Disables instruments within Performance Schema 
matching the input pattern.

Parameters
-----------

in_pattern (VARCHAR(128)):
  A LIKE pattern match (using "%in_pattern%") of events to disable

Example
-----------

To disable all mutex instruments:

mysql> CALL sys.ps_setup_disable_instrument(\'wait/synch/mutex\');
+--------------------------+
| summary                  |
+--------------------------+
| Disabled 155 instruments |
+--------------------------+
1 row in set (0.02 sec)

To disable just a specific TCP/IP based network IO instrument:

mysql> CALL sys.ps_setup_disable_instrument(\'wait/io/socket/sql/server_tcpip_socket\');
+------------------------+
| summary                |
+------------------------+
| Disabled 1 instruments |
+------------------------+
1 row in set (0.00 sec)

To disable all instruments:

mysql> CALL sys.ps_setup_disable_instrument(\'\');
+--------------------------+
| summary                  |
+--------------------------+
| Disabled 547 instruments |
+--------------------------+
1 row in set (0.01 sec)
'
    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    MODIFIES SQL DATA
BEGIN
    UPDATE performance_schema.setup_instruments
       SET enabled = 'NO', timed = 'NO'
     WHERE name LIKE CONCAT('%', in_pattern, '%');

    SELECT CONCAT('Disabled ', @rows := ROW_COUNT(), ' instrument', IF(@rows != 1, 's', '')) AS summary;
END$$

DELIMITER ;
