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

DROP PROCEDURE IF EXISTS ps_setup_show_enabled_consumers;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE ps_setup_show_enabled_consumers ()
    COMMENT '
Description
-----------

Shows all currently enabled consumers.

Parameters
-----------

None

Example
-----------

mysql> CALL sys.ps_setup_show_enabled_consumers();

+---------------------------+
| enabled_consumers         |
+---------------------------+
| events_statements_current |
| global_instrumentation    |
| thread_instrumentation    |
| statements_digest         |
+---------------------------+
4 rows in set (0.05 sec)
'
    SQL SECURITY INVOKER
    DETERMINISTIC
    READS SQL DATA
BEGIN
    SELECT name AS enabled_consumers
      FROM performance_schema.setup_consumers
     WHERE enabled = 'YES'
     ORDER BY enabled_consumers;
END$$

DELIMITER ;
