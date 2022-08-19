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

DROP PROCEDURE IF EXISTS ps_setup_reset_to_default;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE ps_setup_reset_to_default (
       IN in_verbose BOOLEAN
    )
    COMMENT '
Description
-----------

Resets the Performance Schema setup to the default settings.

Parameters
-----------

in_verbose (BOOLEAN):
  Whether to print each setup stage (including the SQL) whilst running.

Example
-----------

mysql> CALL sys.ps_setup_reset_to_default(true)\\G
*************************** 1. row ***************************
status: Resetting: setup_actors
DELETE
FROM performance_schema.setup_actors
 WHERE NOT (HOST = \'%\' AND USER = \'%\' AND `ROLE` = \'%\')
1 row in set (0.00 sec)

*************************** 1. row ***************************
status: Resetting: setup_actors
INSERT IGNORE INTO performance_schema.setup_actors
VALUES (\'%\', \'%\', \'%\')
1 row in set (0.00 sec)
...

mysql> CALL sys.ps_setup_reset_to_default(false)\\G
Query OK, 0 rows affected (0.00 sec)
'
    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    MODIFIES SQL DATA
BEGIN
    SET @query = 'DELETE
                    FROM performance_schema.setup_actors
                   WHERE NOT (HOST = ''%'' AND USER = ''%'' AND `ROLE` = ''%'')';

    IF (in_verbose) THEN
        SELECT CONCAT('Resetting: setup_actors\n', REPLACE(@query, '  ', '')) AS status;
    END IF;

    PREPARE reset_stmt FROM @query;
    EXECUTE reset_stmt;
    DEALLOCATE PREPARE reset_stmt;

    SET @query = 'INSERT IGNORE INTO performance_schema.setup_actors
                  VALUES (''%'', ''%'', ''%'', ''YES'', ''YES'')';

    IF (in_verbose) THEN
        SELECT CONCAT('Resetting: setup_actors\n', REPLACE(@query, '  ', '')) AS status;
    END IF;

    PREPARE reset_stmt FROM @query;
    EXECUTE reset_stmt;
    DEALLOCATE PREPARE reset_stmt;

    SET @query = 'UPDATE performance_schema.setup_instruments
                     SET ENABLED = sys.ps_is_instrument_default_enabled(NAME),
                         TIMED   = sys.ps_is_instrument_default_timed(NAME)';

    IF (in_verbose) THEN
        SELECT CONCAT('Resetting: setup_instruments\n', REPLACE(@query, '  ', '')) AS status;
    END IF;

    PREPARE reset_stmt FROM @query;
    EXECUTE reset_stmt;
    DEALLOCATE PREPARE reset_stmt;

    SET @query = 'UPDATE performance_schema.setup_consumers
                     SET ENABLED = IF(NAME IN (''events_statements_current'', ''events_transactions_current'', ''global_instrumentation'', ''thread_instrumentation'', ''statements_digest''), ''YES'', ''NO'')';

    IF (in_verbose) THEN
        SELECT CONCAT('Resetting: setup_consumers\n', REPLACE(@query, '  ', '')) AS status;
    END IF;

    PREPARE reset_stmt FROM @query;
    EXECUTE reset_stmt;
    DEALLOCATE PREPARE reset_stmt;

    SET @query = 'DELETE
                    FROM performance_schema.setup_objects
                   WHERE NOT (OBJECT_TYPE IN (''EVENT'', ''FUNCTION'', ''PROCEDURE'', ''TABLE'', ''TRIGGER'') AND OBJECT_NAME = ''%'' 
                     AND (OBJECT_SCHEMA = ''mysql''              AND ENABLED = ''NO''  AND TIMED = ''NO'' )
                      OR (OBJECT_SCHEMA = ''performance_schema'' AND ENABLED = ''NO''  AND TIMED = ''NO'' )
                      OR (OBJECT_SCHEMA = ''information_schema'' AND ENABLED = ''NO''  AND TIMED = ''NO'' )
                      OR (OBJECT_SCHEMA = ''%''                  AND ENABLED = ''YES'' AND TIMED = ''YES''))';

    IF (in_verbose) THEN
        SELECT CONCAT('Resetting: setup_objects\n', REPLACE(@query, '  ', '')) AS status;
    END IF;

    PREPARE reset_stmt FROM @query;
    EXECUTE reset_stmt;
    DEALLOCATE PREPARE reset_stmt;

    SET @query = 'INSERT IGNORE INTO performance_schema.setup_objects
                  VALUES (''EVENT''    , ''mysql''             , ''%'', ''NO'' , ''NO'' ),
                         (''EVENT''    , ''performance_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''EVENT''    , ''information_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''EVENT''    , ''%''                 , ''%'', ''YES'', ''YES''),
                         (''FUNCTION'' , ''mysql''             , ''%'', ''NO'' , ''NO'' ),
                         (''FUNCTION'' , ''performance_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''FUNCTION'' , ''information_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''FUNCTION'' , ''%''                 , ''%'', ''YES'', ''YES''),
                         (''PROCEDURE'', ''mysql''             , ''%'', ''NO'' , ''NO'' ),
                         (''PROCEDURE'', ''performance_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''PROCEDURE'', ''information_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''PROCEDURE'', ''%''                 , ''%'', ''YES'', ''YES''),
                         (''TABLE''    , ''mysql''             , ''%'', ''NO'' , ''NO'' ),
                         (''TABLE''    , ''performance_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''TABLE''    , ''information_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''TABLE''    , ''%''                 , ''%'', ''YES'', ''YES''),
                         (''TRIGGER''  , ''mysql''             , ''%'', ''NO'' , ''NO'' ),
                         (''TRIGGER''  , ''performance_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''TRIGGER''  , ''information_schema'', ''%'', ''NO'' , ''NO'' ),
                         (''TRIGGER''  , ''%''                 , ''%'', ''YES'', ''YES'')';

    IF (in_verbose) THEN
        SELECT CONCAT('Resetting: setup_objects\n', REPLACE(@query, '  ', '')) AS status;
    END IF;

    PREPARE reset_stmt FROM @query;
    EXECUTE reset_stmt;
    DEALLOCATE PREPARE reset_stmt;

    SET @query = 'UPDATE performance_schema.threads
                     SET INSTRUMENTED = ''YES''';

    IF (in_verbose) THEN
        SELECT CONCAT('Resetting: threads\n', REPLACE(@query, '  ', '')) AS status;
    END IF;

    PREPARE reset_stmt FROM @query;
    EXECUTE reset_stmt;
    DEALLOCATE PREPARE reset_stmt;
END$$

DELIMITER ;

