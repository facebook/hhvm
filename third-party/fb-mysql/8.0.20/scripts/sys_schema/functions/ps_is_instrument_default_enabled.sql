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

DROP FUNCTION IF EXISTS ps_is_instrument_default_enabled;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' FUNCTION ps_is_instrument_default_enabled (
        in_instrument VARCHAR(128)
    ) 
    RETURNS ENUM('YES', 'NO')
    COMMENT '
Description
-----------

Returns whether an instrument is enabled by default in this version of MySQL.

Parameters
-----------

in_instrument VARCHAR(128): 
  The instrument to check.

Returns
-----------

ENUM(\'YES\', \'NO\')

Example
-----------

mysql> SELECT sys.ps_is_instrument_default_enabled(\'statement/sql/select\');
+--------------------------------------------------------------+
| sys.ps_is_instrument_default_enabled(\'statement/sql/select\') |
+--------------------------------------------------------------+
| YES                                                          |
+--------------------------------------------------------------+
1 row in set (0.00 sec)
'
    SQL SECURITY INVOKER
    DETERMINISTIC 
    READS SQL DATA 
BEGIN
    DECLARE v_enabled ENUM('YES', 'NO');

    IF (in_instrument LIKE 'stage/%') THEN
    BEGIN
      /* Stages are enabled by default if the progress property is set. */
      SET v_enabled = (SELECT
                        IF(find_in_set("progress", PROPERTIES) != 0, 'YES', 'NO')
                        FROM performance_schema.setup_instruments
                        WHERE NAME = in_instrument);
      SET v_enabled = IFNULL(v_enabled, 'NO');
    END;
    ELSE
      SET v_enabled = IF(in_instrument LIKE 'wait/synch/%'
                         OR in_instrument LIKE 'wait/io/socket/%'
                        ,
                         'NO',
                         'YES'
                      );
    END IF;

    RETURN v_enabled;
END$$


DELIMITER ;
