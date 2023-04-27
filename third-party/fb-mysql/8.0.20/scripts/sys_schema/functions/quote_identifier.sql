-- Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.
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

DROP FUNCTION IF EXISTS quote_identifier;

DELIMITER $$

-- https://dev.mysql.com/doc/refman/5.7/en/identifiers.html
-- Maximum supported length for any of the current identifiers in 5.7.5+ is 256 characters.
-- Before that, user variables could have any length.
--
-- Based on Paul Dubois' suggestion in Bug #78823/Bug #22011361.
CREATE DEFINER='mysql.sys'@'localhost' FUNCTION quote_identifier(in_identifier TEXT)
    RETURNS TEXT CHARSET UTF8MB4
    COMMENT '
Description
-----------

Takes an unquoted identifier (schema name, table name, etc.) and
returns the identifier quoted with backticks.

Parameters
-----------

in_identifier (TEXT):
  The identifier to quote.

Returns
-----------

TEXT CHARSET UTF8MB4

Example
-----------

mysql> SELECT sys.quote_identifier(''my_identifier'') AS Identifier;
+-----------------+
| Identifier      |
+-----------------+
| `my_identifier` |
+-----------------+
1 row in set (0.00 sec)

mysql> SELECT sys.quote_identifier(''my`idenfier'') AS Identifier;
+----------------+
| Identifier     |
+----------------+
| `my``idenfier` |
+----------------+
1 row in set (0.00 sec)
'
    SQL SECURITY INVOKER
    DETERMINISTIC
    NO SQL
BEGIN
    RETURN CONCAT('`', REPLACE(in_identifier, '`', '``'), '`');
END$$

DELIMITER ;
