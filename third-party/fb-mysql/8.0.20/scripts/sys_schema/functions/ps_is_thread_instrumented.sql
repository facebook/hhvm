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

DROP FUNCTION IF EXISTS ps_is_thread_instrumented;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' FUNCTION ps_is_thread_instrumented (
        in_connection_id BIGINT UNSIGNED
    ) RETURNS ENUM('YES', 'NO', 'UNKNOWN')
    COMMENT '
Description
-----------

Checks whether the provided connection id is instrumented within Performance Schema.

Parameters
-----------

in_connection_id (BIGINT UNSIGNED):
  The id of the connection to check.

Returns
-----------

ENUM(\'YES\', \'NO\', \'UNKNOWN\')

Example
-----------

mysql> SELECT sys.ps_is_thread_instrumented(CONNECTION_ID());
+------------------------------------------------+
| sys.ps_is_thread_instrumented(CONNECTION_ID()) |
+------------------------------------------------+
| YES                                            |
+------------------------------------------------+
'

    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    READS SQL DATA
BEGIN
    DECLARE v_enabled ENUM('YES', 'NO', 'UNKNOWN');

    IF (in_connection_id IS NULL) THEN
        RETURN NULL;
    END IF;

    SELECT INSTRUMENTED INTO v_enabled
      FROM performance_schema.threads 
     WHERE PROCESSLIST_ID = in_connection_id;

    IF (v_enabled IS NULL) THEN
        RETURN 'UNKNOWN';
    ELSE
        RETURN v_enabled;
    END IF;
END$$

DELIMITER ;
