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

DROP FUNCTION IF EXISTS ps_thread_account;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' FUNCTION ps_thread_account (
        in_thread_id BIGINT UNSIGNED
    ) RETURNS TEXT
    COMMENT '
Description
-----------

Return the user@host account for the given Performance Schema thread id.

Parameters
-----------

in_thread_id (BIGINT UNSIGNED):
  The id of the thread to return the account for.

Example
-----------

mysql> select thread_id, processlist_user, processlist_host from performance_schema.threads where type = ''foreground'';
+-----------+------------------+------------------+
| thread_id | processlist_user | processlist_host |
+-----------+------------------+------------------+
|        23 | NULL             | NULL             |
|        30 | root             | localhost        |
|        31 | msandbox         | localhost        |
|        32 | msandbox         | localhost        |
+-----------+------------------+------------------+
4 rows in set (0.00 sec)

mysql> select sys.ps_thread_account(31);
+---------------------------+
| sys.ps_thread_account(31) |
+---------------------------+
| msandbox@localhost        |
+---------------------------+
1 row in set (0.00 sec)
'

    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    READS SQL DATA
BEGIN
    RETURN (SELECT IF(
                      type = 'FOREGROUND',
                      CONCAT(processlist_user, '@', processlist_host),
                      type
                     ) AS account
              FROM `performance_schema`.`threads`
             WHERE thread_id = in_thread_id);
END$$

DELIMITER ;
