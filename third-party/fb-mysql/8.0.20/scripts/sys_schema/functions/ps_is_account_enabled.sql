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

DROP FUNCTION IF EXISTS ps_is_account_enabled;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' FUNCTION ps_is_account_enabled (
        in_host VARCHAR(255), 
        in_user VARCHAR(32)
    ) 
    RETURNS ENUM('YES', 'NO')
    COMMENT '
Description
-----------

Determines whether instrumentation of an account is enabled 
within Performance Schema.

Parameters
-----------

in_host VARCHAR(255): 
  The hostname of the account to check.
in_user VARCHAR(32):
  The username of the account to check.

Returns
-----------

ENUM(\'YES\', \'NO\', \'PARTIAL\')

Example
-----------

mysql> SELECT sys.ps_is_account_enabled(\'localhost\', \'root\');
+------------------------------------------------+
| sys.ps_is_account_enabled(\'localhost\', \'root\') |
+------------------------------------------------+
| YES                                            |
+------------------------------------------------+
1 row in set (0.01 sec)
'
    SQL SECURITY INVOKER
    DETERMINISTIC 
    READS SQL DATA 
BEGIN
    RETURN IF(EXISTS(SELECT 1
                       FROM performance_schema.setup_actors
                      WHERE (`HOST` = '%' OR in_host LIKE `HOST`)
                        AND (`USER` = '%' OR `USER` = in_user)
                        AND (`ENABLED` = 'YES')
                    ),
              'YES', 'NO'
           );
END$$

DELIMITER ;
