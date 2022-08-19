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

DROP FUNCTION IF EXISTS format_statement;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' FUNCTION format_statement (
        statement LONGTEXT
    )
    RETURNS LONGTEXT
    COMMENT '
Description
-----------

Formats a normalized statement, truncating it if it is > 64 characters long by default.

To configure the length to truncate the statement to by default, update the `statement_truncate_len`
variable with `sys_config` table to a different value. Alternatively, to change it just for just 
your particular session, use `SET @sys.statement_truncate_len := <some new value>`.

Useful for printing statement related data from Performance Schema from 
the command line.

Parameters
-----------

statement (LONGTEXT): 
  The statement to format.

Returns
-----------

LONGTEXT

Example
-----------

mysql> SELECT sys.format_statement(digest_text)
    ->   FROM performance_schema.events_statements_summary_by_digest
    ->  ORDER by sum_timer_wait DESC limit 5;
+-------------------------------------------------------------------+
| sys.format_statement(digest_text)                                 |
+-------------------------------------------------------------------+
| CREATE SQL SECURITY INVOKER VI ... KE ? AND `variable_value` > ?  |
| CREATE SQL SECURITY INVOKER VI ... ait` IS NOT NULL , `esc` . ... |
| CREATE SQL SECURITY INVOKER VI ... ait` IS NOT NULL , `sys` . ... |
| CREATE SQL SECURITY INVOKER VI ...  , `compressed_size` ) ) DESC  |
| CREATE SQL SECURITY INVOKER VI ... LIKE ? ORDER BY `timer_start`  |
+-------------------------------------------------------------------+
5 rows in set (0.00 sec)
'
    SQL SECURITY INVOKER
    DETERMINISTIC
    NO SQL
BEGIN
  -- Check if we have the configured length, if not, init it
  IF @sys.statement_truncate_len IS NULL THEN
      SET @sys.statement_truncate_len = sys_get_config('statement_truncate_len', 64);
  END IF;

  IF CHAR_LENGTH(statement) > @sys.statement_truncate_len THEN
      RETURN REPLACE(CONCAT(LEFT(statement, (@sys.statement_truncate_len/2)-2), ' ... ', RIGHT(statement, (@sys.statement_truncate_len/2)-2)), '\n', ' ');
  ELSE 
      RETURN REPLACE(statement, '\n', ' ');
  END IF;
END$$

DELIMITER ;
