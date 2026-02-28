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

DROP FUNCTION IF EXISTS extract_schema_from_file_name;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' FUNCTION extract_schema_from_file_name (
        path VARCHAR(512)
    )
    RETURNS VARCHAR(64) 
    COMMENT '
Description
-----------

Takes a raw file path, and attempts to extract the schema name from it.

Useful for when interacting with Performance Schema data 
concerning IO statistics, for example.

Currently relies on the fact that a table data file will be within a 
specified database directory (will not work with partitions or tables
that specify an individual DATA_DIRECTORY).

Parameters
-----------

path (VARCHAR(512)):
  The full file path to a data file to extract the schema name from.

Returns
-----------

VARCHAR(64)

Example
-----------

mysql> SELECT sys.extract_schema_from_file_name(\'/var/lib/mysql/employees/employee.ibd\');
+----------------------------------------------------------------------------+
| sys.extract_schema_from_file_name(\'/var/lib/mysql/employees/employee.ibd\') |
+----------------------------------------------------------------------------+
| employees                                                                  |
+----------------------------------------------------------------------------+
1 row in set (0.00 sec)
'
    SQL SECURITY INVOKER
    DETERMINISTIC
    NO SQL
BEGIN
    RETURN LEFT(SUBSTRING_INDEX(SUBSTRING_INDEX(REPLACE(path, '\\', '/'), '/', -2), '/', 1), 64);
END$$

DELIMITER ;
