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

DROP PROCEDURE IF EXISTS statement_performance_analyzer;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE statement_performance_analyzer (
        IN in_action ENUM('snapshot', 'overall', 'delta', 'create_table', 'create_tmp', 'save', 'cleanup'),
        IN in_table VARCHAR(129),
        IN in_views SET ('with_runtimes_in_95th_percentile', 'analysis', 'with_errors_or_warnings', 'with_full_table_scans', 'with_sorting', 'with_temp_tables', 'custom')
    )
    COMMENT '
Description
-----------

Create a report of the statements running on the server.

The views are calculated based on the overall and/or delta activity.

Requires the SUPER privilege for "SET sql_log_bin = 0;".

Parameters
-----------

in_action (ENUM(''snapshot'', ''overall'', ''delta'', ''create_tmp'', ''create_table'', ''save'', ''cleanup'')):
  The action to take. Supported actions are:
    * snapshot      Store a snapshot. The default is to make a snapshot of the current content of
                    performance_schema.events_statements_summary_by_digest, but by setting in_table
                    this can be overwritten to copy the content of the specified table.
                    The snapshot is stored in the sys.tmp_digests temporary table.
    * overall       Generate analyzis based on the content specified by in_table. For the overall analyzis,
                    in_table can be NOW() to use a fresh snapshot. This will overwrite an existing snapshot.
                    Use NULL for in_table to use the existing snapshot. If in_table IS NULL and no snapshot
                    exists, a new will be created.
                    See also in_views and @sys.statement_performance_analyzer.limit.
    * delta         Generate a delta analysis. The delta will be calculated between the reference table in
                    in_table and the snapshot. An existing snapshot must exist.
                    The action uses the sys.tmp_digests_delta temporary table.
                    See also in_views and @sys.statement_performance_analyzer.limit.
    * create_table  Create a regular table suitable for storing the snapshot for later use, e.g. for
                    calculating deltas.
    * create_tmp    Create a temporary table suitable for storing the snapshot for later use, e.g. for
                    calculating deltas.
    * save          Save the snapshot in the table specified by in_table. The table must exists and have
                    the correct structure.
                    If no snapshot exists, a new is created.
    * cleanup       Remove the temporary tables used for the snapshot and delta.

in_table (VARCHAR(129)):
  The table argument used for some actions. Use the format ''db1.t1'' or ''t1'' without using any backticks (`)
  for quoting. Periods (.) are not supported in the database and table names.

  The meaning of the table for each action supporting the argument is:

    * snapshot      The snapshot is created based on the specified table. Set to NULL or NOW() to use
                    the current content of performance_schema.events_statements_summary_by_digest.
    * overall       The table with the content to create the overall analyzis for. The following values
                    can be used:
                      - A table name - use the content of that table.
                      - NOW()        - create a fresh snapshot and overwrite the existing snapshot.
                      - NULL         - use the last stored snapshot.
    * delta         The table name is mandatory and specified the reference view to compare the currently
                    stored snapshot against. If no snapshot exists, a new will be created.
    * create_table  The name of the regular table to create.
    * create_tmp    The name of the temporary table to create.
    * save          The name of the table to save the currently stored snapshot into.

in_views (SET (''with_runtimes_in_95th_percentile'', ''analysis'', ''with_errors_or_warnings'',
               ''with_full_table_scans'', ''with_sorting'', ''with_temp_tables'', ''custom''))
  Which views to include:

    * with_runtimes_in_95th_percentile  Based on the sys.statements_with_runtimes_in_95th_percentile view
    * analysis                          Based on the sys.statement_analysis view
    * with_errors_or_warnings           Based on the sys.statements_with_errors_or_warnings view
    * with_full_table_scans             Based on the sys.statements_with_full_table_scans view
    * with_sorting                      Based on the sys.statements_with_sorting view
    * with_temp_tables                  Based on the sys.statements_with_temp_tables view
    * custom                            Use a custom view. This view must be specified in @sys.statement_performance_analyzer.view to an existing view or a query

Default is to include all except ''custom''.


Configuration Options
----------------------

sys.statement_performance_analyzer.limit
  The maximum number of rows to include for the views that does not have a built-in limit (e.g. the 95th percentile view).
  If not set the limit is 100.

sys.statement_performance_analyzer.view
  Used together with the ''custom'' view. If the value contains a space, it is considered a query, otherwise it must be
  an existing view querying the performance_schema.events_statements_summary_by_digest table. There cannot be any limit
  clause including in the query or view definition if @sys.statement_performance_analyzer.limit > 0.
  If specifying a view, use the same format as for in_table.

sys.debug
  Whether to provide debugging output.
  Default is ''OFF''. Set to ''ON'' to include.


Example
--------

To create a report with the queries in the 95th percentile since last truncate of performance_schema.events_statements_summary_by_digest
and the delta for a 1 minute period:

   1. Create a temporary table to store the initial snapshot.
   2. Create the initial snapshot.
   3. Save the initial snapshot in the temporary table.
   4. Wait one minute.
   5. Create a new snapshot.
   6. Perform analyzis based on the new snapshot.
   7. Perform analyzis based on the delta between the initial and new snapshots.

mysql> CALL sys.statement_performance_analyzer(''create_tmp'', ''mydb.tmp_digests_ini'', NULL);
Query OK, 0 rows affected (0.08 sec)

mysql> CALL sys.statement_performance_analyzer(''snapshot'', NULL, NULL);
Query OK, 0 rows affected (0.02 sec)

mysql> CALL sys.statement_performance_analyzer(''save'', ''mydb.tmp_digests_ini'', NULL);
Query OK, 0 rows affected (0.00 sec)

mysql> DO SLEEP(60);
Query OK, 0 rows affected (1 min 0.00 sec)

mysql> CALL sys.statement_performance_analyzer(''snapshot'', NULL, NULL);
Query OK, 0 rows affected (0.02 sec)

mysql> CALL sys.statement_performance_analyzer(''overall'', NULL, ''with_runtimes_in_95th_percentile'');
+-----------------------------------------+
| Next Output                             |
+-----------------------------------------+
| Queries with Runtime in 95th Percentile |
+-----------------------------------------+
1 row in set (0.05 sec)

...

mysql> CALL sys.statement_performance_analyzer(''delta'', ''mydb.tmp_digests_ini'', ''with_runtimes_in_95th_percentile'');
+-----------------------------------------+
| Next Output                             |
+-----------------------------------------+
| Queries with Runtime in 95th Percentile |
+-----------------------------------------+
1 row in set (0.03 sec)

...


To create an overall report of the 95th percentile queries and the top 10 queries with full table scans:

mysql> CALL sys.statement_performance_analyzer(''snapshot'', NULL, NULL);
Query OK, 0 rows affected (0.01 sec)

mysql> SET @sys.statement_performance_analyzer.limit = 10;
Query OK, 0 rows affected (0.00 sec)

mysql> CALL sys.statement_performance_analyzer(''overall'', NULL, ''with_runtimes_in_95th_percentile,with_full_table_scans'');
+-----------------------------------------+
| Next Output                             |
+-----------------------------------------+
| Queries with Runtime in 95th Percentile |
+-----------------------------------------+
1 row in set (0.01 sec)

...

+-------------------------------------+
| Next Output                         |
+-------------------------------------+
| Top 10 Queries with Full Table Scan |
+-------------------------------------+
1 row in set (0.09 sec)

...


Use a custom view showing the top 10 query sorted by total execution time refreshing the view every minute using
the watch command in Linux.

mysql> CREATE OR REPLACE VIEW mydb.my_statements AS
    -> SELECT sys.format_statement(DIGEST_TEXT) AS query,
    ->        SCHEMA_NAME AS db,
    ->        COUNT_STAR AS exec_count,
    ->        format_pico_time(SUM_TIMER_WAIT) AS total_latency,
    ->        format_pico_time(AVG_TIMER_WAIT) AS avg_latency,
    ->        ROUND(IFNULL(SUM_ROWS_SENT / NULLIF(COUNT_STAR, 0), 0)) AS rows_sent_avg,
    ->        ROUND(IFNULL(SUM_ROWS_EXAMINED / NULLIF(COUNT_STAR, 0), 0)) AS rows_examined_avg,
    ->        ROUND(IFNULL(SUM_ROWS_AFFECTED / NULLIF(COUNT_STAR, 0), 0)) AS rows_affected_avg,
    ->        DIGEST AS digest
    ->   FROM performance_schema.events_statements_summary_by_digest
    -> ORDER BY SUM_TIMER_WAIT DESC;
Query OK, 0 rows affected (0.01 sec)

mysql> CALL sys.statement_performance_analyzer(''create_table'', ''mydb.digests_prev'', NULL);
Query OK, 0 rows affected (0.10 sec)

shell$ watch -n 60 "mysql sys --table -e \"
> SET @sys.statement_performance_analyzer.view = ''mydb.my_statements'';
> SET @sys.statement_performance_analyzer.limit = 10;
> CALL statement_performance_analyzer(''snapshot'', NULL, NULL);
> CALL statement_performance_analyzer(''delta'', ''mydb.digests_prev'', ''custom'');
> CALL statement_performance_analyzer(''save'', ''mydb.digests_prev'', NULL);
> \""

Every 60.0s: mysql sys --table -e "                                                                                                   ...  Mon Dec 22 10:58:51 2014

+----------------------------------+
| Next Output                      |
+----------------------------------+
| Top 10 Queries Using Custom View |
+----------------------------------+
+-------------------+-------+------------+---------------+-------------+---------------+-------------------+-------------------+----------------------------------+
| query             | db    | exec_count | total_latency | avg_latency | rows_sent_avg | rows_examined_avg | rows_affected_avg | digest                           |
+-------------------+-------+------------+---------------+-------------+---------------+-------------------+-------------------+----------------------------------+
...
'
    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    CONTAINS SQL
BEGIN
    DECLARE v_table_exists, v_tmp_digests_table_exists, v_custom_view_exists ENUM('', 'BASE TABLE', 'VIEW', 'TEMPORARY') DEFAULT '';
    DECLARE v_this_thread_enabled ENUM('YES', 'NO');
    DECLARE v_force_new_snapshot BOOLEAN DEFAULT FALSE;
    DECLARE v_digests_table VARCHAR(133);
    DECLARE v_quoted_table, v_quoted_custom_view VARCHAR(133) DEFAULT '';
    DECLARE v_table_db, v_table_name, v_custom_db, v_custom_name VARCHAR(64);
    DECLARE v_digest_table_template, v_checksum_ref, v_checksum_table text;
    DECLARE v_sql longtext;
    -- Maximum supported length for MESSAGE_TEXT with the SIGNAL command is 128 chars.
    DECLARE v_error_msg VARCHAR(128);
    DECLARE v_old_group_concat_max_len INT UNSIGNED DEFAULT 0;


    -- Don't instrument this thread
    SELECT INSTRUMENTED INTO v_this_thread_enabled FROM performance_schema.threads WHERE PROCESSLIST_ID = CONNECTION_ID();
    IF (v_this_thread_enabled = 'YES') THEN
        CALL sys.ps_setup_disable_thread(CONNECTION_ID());
    END IF;

    -- Temporary table are used - disable sql_log_bin if necessary to prevent them replicating
    SET @log_bin := @@sql_log_bin;
    IF (@log_bin = 1) THEN
        SET sql_log_bin = 0;
    END IF;


    -- Set configuration options
    IF (@sys.statement_performance_analyzer.limit IS NULL) THEN
        SET @sys.statement_performance_analyzer.limit = sys.sys_get_config('statement_performance_analyzer.limit', '100');
    END IF;
    IF (@sys.debug IS NULL) THEN
        SET @sys.debug                                = sys.sys_get_config('debug'                               , 'OFF');
    END IF;


    -- If in_table is set, break in_table into a db and table component and check whether it exists
    -- in_table = NOW() is considered like it's not set.
    IF (in_table = 'NOW()') THEN
        SET v_force_new_snapshot = TRUE,
            in_table             = NULL;
    ELSEIF (in_table IS NOT NULL) THEN
        IF (NOT INSTR(in_table, '.')) THEN
            -- No . in the table name - use current database
            -- DATABASE() will be the database of the procedure
            SET v_table_db   = DATABASE(),
                v_table_name = in_table;
        ELSE
            SET v_table_db   = SUBSTRING_INDEX(in_table, '.', 1);
            SET v_table_name = SUBSTRING(in_table, CHAR_LENGTH(v_table_db)+2);
        END IF;

        SET v_quoted_table = CONCAT('`', v_table_db, '`.`', v_table_name, '`');

        IF (@sys.debug = 'ON') THEN
            SELECT CONCAT('in_table is: db = ''', v_table_db, ''', table = ''', v_table_name, '''') AS 'Debug';
        END IF;

        IF (v_table_db = DATABASE() AND (v_table_name = 'tmp_digests' OR v_table_name = 'tmp_digests_delta')) THEN
            SET v_error_msg = CONCAT('Invalid value for in_table: ', v_quoted_table, ' is reserved table name.');
            SIGNAL SQLSTATE '45000'
               SET MESSAGE_TEXT = v_error_msg;
        END IF;

        CALL sys.table_exists(v_table_db, v_table_name, v_table_exists);
        IF (@sys.debug = 'ON') THEN
            SELECT CONCAT('v_table_exists = ', v_table_exists) AS 'Debug';
        END IF;

        IF (v_table_exists = 'BASE TABLE') THEN
            SET v_old_group_concat_max_len = @@session.group_concat_max_len;
            SET @@session.group_concat_max_len = 2048;
            -- Verify that the table has the correct table definition
            -- This can only be done for base tables as temporary aren't in information_schema.COLUMNS.
            -- This also minimises the risk of using a production table.
            SET v_checksum_ref = (
                 SELECT GROUP_CONCAT(CONCAT(COLUMN_NAME, COLUMN_TYPE) ORDER BY ORDINAL_POSITION) AS Checksum
                   FROM information_schema.COLUMNS
                  WHERE TABLE_SCHEMA = 'performance_schema' AND TABLE_NAME = 'events_statements_summary_by_digest'
                ),
                v_checksum_table = (
                 SELECT GROUP_CONCAT(CONCAT(COLUMN_NAME, COLUMN_TYPE) ORDER BY ORDINAL_POSITION) AS Checksum
                   FROM information_schema.COLUMNS
                  WHERE TABLE_SCHEMA = v_table_db AND TABLE_NAME = v_table_name
                );
            SET @@session.group_concat_max_len = v_old_group_concat_max_len;
            IF (v_checksum_ref <> v_checksum_table) THEN
                -- The table does not have the correct definition, so abandon
                SET v_error_msg = CONCAT('The table ',
                                         IF(CHAR_LENGTH(v_quoted_table) > 93, CONCAT('...', SUBSTRING(v_quoted_table, -90)), v_quoted_table),
                                         ' has the wrong definition.');
                SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = v_error_msg;
            END IF;
        END IF;
    END IF;


    IF (in_views IS NULL OR in_views = '') THEN
        -- Set to default
        SET in_views = 'with_runtimes_in_95th_percentile,analysis,with_errors_or_warnings,with_full_table_scans,with_sorting,with_temp_tables';
    END IF;


    -- Validate settings
    CALL sys.table_exists(DATABASE(), 'tmp_digests', v_tmp_digests_table_exists);
    IF (@sys.debug = 'ON') THEN
        SELECT CONCAT('v_tmp_digests_table_exists = ', v_tmp_digests_table_exists) AS 'Debug';
    END IF;

    CASE
        WHEN in_action IN ('snapshot', 'overall') THEN
            -- in_table must be NULL, NOW(), or an existing table
            IF (in_table IS NOT NULL) THEN
                IF (NOT v_table_exists IN ('TEMPORARY', 'BASE TABLE')) THEN
                    SET v_error_msg = CONCAT('The ', in_action, ' action requires in_table to be NULL, NOW() or specify an existing table.',
                                             ' The table ',
                                             IF(CHAR_LENGTH(v_quoted_table) > 16, CONCAT('...', SUBSTRING(v_quoted_table, -13)), v_quoted_table),
                                             ' does not exist.');
                    SIGNAL SQLSTATE '45000'
                       SET MESSAGE_TEXT = v_error_msg;
                END IF;
            END IF;

        WHEN in_action IN ('delta', 'save') THEN
            -- in_table must be an existing table
            IF (v_table_exists NOT IN ('TEMPORARY', 'BASE TABLE')) THEN
                SET v_error_msg = CONCAT('The ', in_action, ' action requires in_table to be an existing table.',
                                         IF(in_table IS NOT NULL, CONCAT(' The table ',
                                             IF(CHAR_LENGTH(v_quoted_table) > 39, CONCAT('...', SUBSTRING(v_quoted_table, -36)), v_quoted_table),
                                             ' does not exist.'), ''));
                SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = v_error_msg;
            END IF;

            IF (in_action = 'delta' AND v_tmp_digests_table_exists <> 'TEMPORARY') THEN
                SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'An existing snapshot generated with the statement_performance_analyzer() must exist.';
            END IF;
        WHEN in_action = 'create_tmp' THEN
            -- in_table must not exists as a temporary table
            IF (v_table_exists = 'TEMPORARY') THEN
                SET v_error_msg = CONCAT('Cannot create the table ',
                                         IF(CHAR_LENGTH(v_quoted_table) > 72, CONCAT('...', SUBSTRING(v_quoted_table, -69)), v_quoted_table),
                                         ' as it already exists.');
                SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = v_error_msg;
            END IF;

        WHEN in_action = 'create_table' THEN
            -- in_table must not exists at all
            IF (v_table_exists <> '') THEN
                SET v_error_msg = CONCAT('Cannot create the table ',
                                         IF(CHAR_LENGTH(v_quoted_table) > 52, CONCAT('...', SUBSTRING(v_quoted_table, -49)), v_quoted_table),
                                         ' as it already exists',
                                         IF(v_table_exists = 'TEMPORARY', ' as a temporary table.', '.'));
                SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = v_error_msg;
            END IF;

        WHEN in_action = 'cleanup' THEN
            -- doesn't use any of the arguments
            DO (SELECT 1);
        ELSE
            SIGNAL SQLSTATE '45000'
               SET MESSAGE_TEXT = 'Unknown action. Supported actions are: cleanup, create_table, create_tmp, delta, overall, save, snapshot';
    END CASE;

    SET v_digest_table_template = 'CREATE %{TEMPORARY}TABLE %{TABLE_NAME} (
  `SCHEMA_NAME` varchar(64) DEFAULT NULL,
  `DIGEST` varchar(64) DEFAULT NULL,
  `DIGEST_TEXT` longtext,
  `COUNT_STAR` bigint unsigned NOT NULL,
  `SUM_TIMER_WAIT` bigint unsigned NOT NULL,
  `MIN_TIMER_WAIT` bigint unsigned NOT NULL,
  `AVG_TIMER_WAIT` bigint unsigned NOT NULL,
  `MAX_TIMER_WAIT` bigint unsigned NOT NULL,
  `SUM_LOCK_TIME` bigint unsigned NOT NULL,
  `SUM_ERRORS` bigint unsigned NOT NULL,
  `SUM_WARNINGS` bigint unsigned NOT NULL,
  `SUM_ROWS_AFFECTED` bigint unsigned NOT NULL,
  `SUM_ROWS_SENT` bigint unsigned NOT NULL,
  `SUM_ROWS_EXAMINED` bigint unsigned NOT NULL,
  `SUM_CREATED_TMP_DISK_TABLES` bigint unsigned NOT NULL,
  `SUM_CREATED_TMP_TABLES` bigint unsigned NOT NULL,
  `SUM_SELECT_FULL_JOIN` bigint unsigned NOT NULL,
  `SUM_SELECT_FULL_RANGE_JOIN` bigint unsigned NOT NULL,
  `SUM_SELECT_RANGE` bigint unsigned NOT NULL,
  `SUM_SELECT_RANGE_CHECK` bigint unsigned NOT NULL,
  `SUM_SELECT_SCAN` bigint unsigned NOT NULL,
  `SUM_SORT_MERGE_PASSES` bigint unsigned NOT NULL,
  `SUM_SORT_RANGE` bigint unsigned NOT NULL,
  `SUM_SORT_ROWS` bigint unsigned NOT NULL,
  `SUM_SORT_SCAN` bigint unsigned NOT NULL,
  `SUM_NO_INDEX_USED` bigint unsigned NOT NULL,
  `SUM_NO_GOOD_INDEX_USED` bigint unsigned NOT NULL,
  `SUM_ROWS_DELETED` bigint unsigned NOT NULL,
  `SUM_ROWS_INSERTED` bigint unsigned NOT NULL,
  `SUM_ROWS_UPDATED` bigint unsigned NOT NULL,
  `SUM_CPU_TIME` bigint unsigned NOT NULL,
  `SUM_TMP_TABLE_BYTES_WRITTEN` bigint(20) unsigned NOT NULL,
  `SUM_FILESORT_BYTES_WRITTEN` bigint(20) unsigned NOT NULL,
  `SUM_INDEX_DIVE_COUNT` bigint(20) unsigned NOT NULL,
  `SUM_INDEX_DIVE_CPU` bigint(20) unsigned NOT NULL,
  `SUM_COMPILATION_CPU` bigint(20) unsigned NOT NULL,
  `SUM_ELAPSED_TIME` bigint(20) unsigned NOT NULL,
  `SUM_SKIPPED` bigint(20) unsigned NOT NULL,
  `SUM_FILESORT_DISK_USAGE` bigint(20) unsigned NOT NULL,
  `SUM_TMP_TABLE_DISK_USAGE` bigint(20) unsigned NOT NULL,
  `FIRST_SEEN` timestamp(6) NULL DEFAULT NULL,
  `LAST_SEEN` timestamp(6) NULL DEFAULT NULL,
  `QUANTILE_95` bigint unsigned NOT NULL,
  `QUANTILE_99` bigint unsigned NOT NULL,
  `QUANTILE_999` bigint unsigned NOT NULL,
  `QUERY_SAMPLE_TEXT` longtext,
  `QUERY_SAMPLE_SEEN` timestamp(6) NULL DEFAULT NULL,
  `QUERY_SAMPLE_TIMER_WAIT` bigint unsigned NOT NULL,
  INDEX (SCHEMA_NAME, DIGEST)
) DEFAULT CHARSET=utf8mb4';

    -- Do the action
    -- The actions snapshot, ... requires a fresh snapshot - create it now
    IF (v_force_new_snapshot
           OR in_action = 'snapshot'
           OR (in_action = 'overall' AND in_table IS NULL)
           OR (in_action = 'save' AND v_tmp_digests_table_exists <> 'TEMPORARY')
       ) THEN
        IF (v_tmp_digests_table_exists = 'TEMPORARY') THEN
            IF (@sys.debug = 'ON') THEN
                SELECT 'DROP TEMPORARY TABLE IF EXISTS tmp_digests' AS 'Debug';
            END IF;
            DROP TEMPORARY TABLE IF EXISTS tmp_digests;
        END IF;
        CALL sys.execute_prepared_stmt(REPLACE(REPLACE(v_digest_table_template, '%{TEMPORARY}', 'TEMPORARY '), '%{TABLE_NAME}', 'tmp_digests'));

        SET v_sql = CONCAT('INSERT INTO tmp_digests SELECT * FROM ',
                           IF(in_table IS NULL OR in_action = 'save', 'performance_schema.events_statements_summary_by_digest', v_quoted_table));
        CALL sys.execute_prepared_stmt(v_sql);
    END IF;

    -- Go through the remaining actions
    IF (in_action IN ('create_table', 'create_tmp')) THEN
        IF (in_action = 'create_table') THEN
            CALL sys.execute_prepared_stmt(REPLACE(REPLACE(v_digest_table_template, '%{TEMPORARY}', ''), '%{TABLE_NAME}', v_quoted_table));
        ELSE
            CALL sys.execute_prepared_stmt(REPLACE(REPLACE(v_digest_table_template, '%{TEMPORARY}', 'TEMPORARY '), '%{TABLE_NAME}', v_quoted_table));
        END IF;
    ELSEIF (in_action = 'save') THEN
        CALL sys.execute_prepared_stmt(CONCAT('DELETE FROM ', v_quoted_table));
        CALL sys.execute_prepared_stmt(CONCAT('INSERT INTO ', v_quoted_table, ' SELECT * FROM tmp_digests'));
    ELSEIF (in_action = 'cleanup') THEN
        DROP TEMPORARY TABLE IF EXISTS sys.tmp_digests;
        DROP TEMPORARY TABLE IF EXISTS sys.tmp_digests_delta;
    ELSEIF (in_action IN ('overall', 'delta')) THEN
        -- These are almost the same - for delta calculate the delta in tmp_digests_delta and use that instead of tmp_digests.
        -- And overall allows overriding the table to use.
        IF (in_action = 'overall') THEN
            IF (in_table IS NULL) THEN
                SET v_digests_table = 'tmp_digests';
            ELSE
                SET v_digests_table = v_quoted_table;
            END IF;
        ELSE
            SET v_digests_table = 'tmp_digests_delta';
            DROP TEMPORARY TABLE IF EXISTS tmp_digests_delta;
            CREATE TEMPORARY TABLE tmp_digests_delta LIKE tmp_digests;
            SET v_sql = CONCAT('INSERT INTO tmp_digests_delta
SELECT `d_end`.`SCHEMA_NAME`,
       `d_end`.`DIGEST`,
       `d_end`.`DIGEST_TEXT`,
       `d_end`.`COUNT_STAR`-IFNULL(`d_start`.`COUNT_STAR`, 0) AS ''COUNT_STAR'',
       `d_end`.`SUM_TIMER_WAIT`-IFNULL(`d_start`.`SUM_TIMER_WAIT`, 0) AS ''SUM_TIMER_WAIT'',
       `d_end`.`MIN_TIMER_WAIT` AS ''MIN_TIMER_WAIT'',
       IFNULL((`d_end`.`SUM_TIMER_WAIT`-IFNULL(`d_start`.`SUM_TIMER_WAIT`, 0))/NULLIF(`d_end`.`COUNT_STAR`-IFNULL(`d_start`.`COUNT_STAR`, 0), 0), 0) AS ''AVG_TIMER_WAIT'',
       `d_end`.`MAX_TIMER_WAIT` AS ''MAX_TIMER_WAIT'',
       `d_end`.`SUM_LOCK_TIME`-IFNULL(`d_start`.`SUM_LOCK_TIME`, 0) AS ''SUM_LOCK_TIME'',
       `d_end`.`SUM_ERRORS`-IFNULL(`d_start`.`SUM_ERRORS`, 0) AS ''SUM_ERRORS'',
       `d_end`.`SUM_WARNINGS`-IFNULL(`d_start`.`SUM_WARNINGS`, 0) AS ''SUM_WARNINGS'',
       `d_end`.`SUM_ROWS_AFFECTED`-IFNULL(`d_start`.`SUM_ROWS_AFFECTED`, 0) AS ''SUM_ROWS_AFFECTED'',
       `d_end`.`SUM_ROWS_SENT`-IFNULL(`d_start`.`SUM_ROWS_SENT`, 0) AS ''SUM_ROWS_SENT'',
       `d_end`.`SUM_ROWS_EXAMINED`-IFNULL(`d_start`.`SUM_ROWS_EXAMINED`, 0) AS ''SUM_ROWS_EXAMINED'',
       `d_end`.`SUM_CREATED_TMP_DISK_TABLES`-IFNULL(`d_start`.`SUM_CREATED_TMP_DISK_TABLES`, 0) AS ''SUM_CREATED_TMP_DISK_TABLES'',
       `d_end`.`SUM_CREATED_TMP_TABLES`-IFNULL(`d_start`.`SUM_CREATED_TMP_TABLES`, 0) AS ''SUM_CREATED_TMP_TABLES'',
       `d_end`.`SUM_SELECT_FULL_JOIN`-IFNULL(`d_start`.`SUM_SELECT_FULL_JOIN`, 0) AS ''SUM_SELECT_FULL_JOIN'',
       `d_end`.`SUM_SELECT_FULL_RANGE_JOIN`-IFNULL(`d_start`.`SUM_SELECT_FULL_RANGE_JOIN`, 0) AS ''SUM_SELECT_FULL_RANGE_JOIN'',
       `d_end`.`SUM_SELECT_RANGE`-IFNULL(`d_start`.`SUM_SELECT_RANGE`, 0) AS ''SUM_SELECT_RANGE'',
       `d_end`.`SUM_SELECT_RANGE_CHECK`-IFNULL(`d_start`.`SUM_SELECT_RANGE_CHECK`, 0) AS ''SUM_SELECT_RANGE_CHECK'',
       `d_end`.`SUM_SELECT_SCAN`-IFNULL(`d_start`.`SUM_SELECT_SCAN`, 0) AS ''SUM_SELECT_SCAN'',
       `d_end`.`SUM_SORT_MERGE_PASSES`-IFNULL(`d_start`.`SUM_SORT_MERGE_PASSES`, 0) AS ''SUM_SORT_MERGE_PASSES'',
       `d_end`.`SUM_SORT_RANGE`-IFNULL(`d_start`.`SUM_SORT_RANGE`, 0) AS ''SUM_SORT_RANGE'',
       `d_end`.`SUM_SORT_ROWS`-IFNULL(`d_start`.`SUM_SORT_ROWS`, 0) AS ''SUM_SORT_ROWS'',
       `d_end`.`SUM_SORT_SCAN`-IFNULL(`d_start`.`SUM_SORT_SCAN`, 0) AS ''SUM_SORT_SCAN'',
       `d_end`.`SUM_NO_INDEX_USED`-IFNULL(`d_start`.`SUM_NO_INDEX_USED`, 0) AS ''SUM_NO_INDEX_USED'',
       `d_end`.`SUM_NO_GOOD_INDEX_USED`-IFNULL(`d_start`.`SUM_NO_GOOD_INDEX_USED`, 0) AS ''SUM_NO_GOOD_INDEX_USED'',
       `d_end`.`SUM_ROWS_DELETED`-IFNULL(`d_start`.`SUM_ROWS_DELETED`, 0) AS ''SUM_ROWS_DELETED'',
       `d_end`.`SUM_ROWS_INSERTED`-IFNULL(`d_start`.`SUM_ROWS_INSERTED`, 0) AS ''SUM_ROWS_INSERTED'',
       `d_end`.`SUM_ROWS_UPDATED`-IFNULL(`d_start`.`SUM_ROWS_UPDATED`, 0) AS ''SUM_ROWS_UPDATED'',
       `d_end`.`SUM_CPU_TIME`-IFNULL(`d_start`.`SUM_CPU_TIME`, 0) AS ''SUM_CPU_TIME'',
       `d_end`.`SUM_TMP_TABLE_BYTES_WRITTEN`-IFNULL(`d_start`.`SUM_TMP_TABLE_BYTES_WRITTEN`, 0) AS ''SUM_TMP_TABLE_BYTES_WRITTEN'',
       `d_end`.`SUM_FILESORT_BYTES_WRITTEN`-IFNULL(`d_start`.`SUM_FILESORT_BYTES_WRITTEN`, 0) AS ''SUM_FILESORT_BYTES_WRITTEN'',
       `d_end`.`SUM_INDEX_DIVE_COUNT`-IFNULL(`d_start`.`SUM_INDEX_DIVE_COUNT`, 0) AS ''SUM_INDEX_DIVE_COUNT'',
       `d_end`.`SUM_INDEX_DIVE_CPU`-IFNULL(`d_start`.`SUM_INDEX_DIVE_CPU`, 0) AS ''SUM_INDEX_DIVE_CPU'',
       `d_end`.`SUM_COMPILATION_CPU`-IFNULL(`d_start`.`SUM_COMPILATION_CPU`, 0) AS ''SUM_COMPILATION_CPU'',
       `d_end`.`SUM_ELAPSED_TIME`-IFNULL(`d_start`.`SUM_ELAPSED_TIME`, 0) AS ''SUM_ELAPSED_TIME'',
       `d_end`.`SUM_SKIPPED`-IFNULL(`d_start`.`SUM_SKIPPED`, 0) AS ''SUM_SKIPPED'',
       `d_end`.`SUM_FILESORT_DISK_USAGE`-IFNULL(`d_start`.`SUM_FILESORT_DISK_USAGE`, 0) AS ''SUM_FILESORT_DISK_USAGE'',
       `d_end`.`SUM_TMP_TABLE_DISK_USAGE`-IFNULL(`d_start`.`SUM_TMP_TABLE_DISK_USAGE`, 0) AS ''SUM_TMP_TABLE_DISK_USAGE'',
       `d_end`.`FIRST_SEEN`,
       `d_end`.`LAST_SEEN`,
       `d_end`.`QUANTILE_95`,
       `d_end`.`QUANTILE_99`,
       `d_end`.`QUANTILE_999`,
       `d_end`.`QUERY_SAMPLE_TEXT`,
       `d_end`.`QUERY_SAMPLE_SEEN`,
       `d_end`.`QUERY_SAMPLE_TIMER_WAIT`
  FROM tmp_digests d_end
       LEFT OUTER JOIN ', v_quoted_table, ' d_start ON `d_start`.`DIGEST` = `d_end`.`DIGEST`
                                                    AND (`d_start`.`SCHEMA_NAME` = `d_end`.`SCHEMA_NAME`
                                                          OR (`d_start`.`SCHEMA_NAME` IS NULL AND `d_end`.`SCHEMA_NAME` IS NULL)
                                                        )
 WHERE `d_end`.`COUNT_STAR`-IFNULL(`d_start`.`COUNT_STAR`, 0) > 0');
            CALL sys.execute_prepared_stmt(v_sql);
        END IF;

        IF (FIND_IN_SET('with_runtimes_in_95th_percentile', in_views)) THEN
            SELECT 'Queries with Runtime in 95th Percentile' AS 'Next Output';

            DROP TEMPORARY TABLE IF EXISTS tmp_digest_avg_latency_distribution1;
            DROP TEMPORARY TABLE IF EXISTS tmp_digest_avg_latency_distribution2;
            DROP TEMPORARY TABLE IF EXISTS tmp_digest_95th_percentile_by_avg_us;

            CREATE TEMPORARY TABLE tmp_digest_avg_latency_distribution1 (
              cnt bigint unsigned NOT NULL,
              avg_us decimal(21,0) NOT NULL,
              PRIMARY KEY (avg_us)
            ) ENGINE=InnoDB;

            SET v_sql = CONCAT('INSERT INTO tmp_digest_avg_latency_distribution1
SELECT COUNT(*) cnt,
       ROUND(avg_timer_wait/1000000) AS avg_us
  FROM ', v_digests_table, '
 GROUP BY avg_us');
            CALL sys.execute_prepared_stmt(v_sql);

            CREATE TEMPORARY TABLE tmp_digest_avg_latency_distribution2 LIKE tmp_digest_avg_latency_distribution1;
            INSERT INTO tmp_digest_avg_latency_distribution2 SELECT * FROM tmp_digest_avg_latency_distribution1;

            CREATE TEMPORARY TABLE tmp_digest_95th_percentile_by_avg_us (
              avg_us decimal(21,0) NOT NULL,
              percentile decimal(46,4) NOT NULL,
              PRIMARY KEY (avg_us)
            ) ENGINE=InnoDB;

            SET v_sql = CONCAT('INSERT INTO tmp_digest_95th_percentile_by_avg_us
SELECT s2.avg_us avg_us,
       IFNULL(SUM(s1.cnt)/NULLIF((SELECT COUNT(*) FROM ', v_digests_table, '), 0), 0) percentile
  FROM tmp_digest_avg_latency_distribution1 AS s1
       JOIN tmp_digest_avg_latency_distribution2 AS s2 ON s1.avg_us <= s2.avg_us
 GROUP BY s2.avg_us
HAVING percentile > 0.95
 ORDER BY percentile
 LIMIT 1');
            CALL sys.execute_prepared_stmt(v_sql);

            SET v_sql =
                REPLACE(
                    REPLACE(
                        (SELECT VIEW_DEFINITION
                           FROM information_schema.VIEWS
                          WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'statements_with_runtimes_in_95th_percentile'
                        ),
                        '`performance_schema`.`events_statements_summary_by_digest`',
                        v_digests_table
                    ),
                    'sys.x$ps_digest_95th_percentile_by_avg_us',
                    '`sys`.`x$ps_digest_95th_percentile_by_avg_us`'
              );
            CALL sys.execute_prepared_stmt(v_sql);

            DROP TEMPORARY TABLE tmp_digest_avg_latency_distribution1;
            DROP TEMPORARY TABLE tmp_digest_avg_latency_distribution2;
            DROP TEMPORARY TABLE tmp_digest_95th_percentile_by_avg_us;
        END IF;

        IF (FIND_IN_SET('analysis', in_views)) THEN
            SELECT CONCAT('Top ', @sys.statement_performance_analyzer.limit, ' Queries Ordered by Total Latency') AS 'Next Output';
            SET v_sql =
                REPLACE(
                    (SELECT VIEW_DEFINITION
                       FROM information_schema.VIEWS
                      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'statement_analysis'
                    ),
                    '`performance_schema`.`events_statements_summary_by_digest`',
                    v_digests_table
                );
            IF (@sys.statement_performance_analyzer.limit > 0) THEN
                SET v_sql = CONCAT(v_sql, ' LIMIT ', @sys.statement_performance_analyzer.limit);
            END IF;
            CALL sys.execute_prepared_stmt(v_sql);
        END IF;

        IF (FIND_IN_SET('with_errors_or_warnings', in_views)) THEN
            SELECT CONCAT('Top ', @sys.statement_performance_analyzer.limit, ' Queries with Errors') AS 'Next Output';
            SET v_sql =
                REPLACE(
                    (SELECT VIEW_DEFINITION
                       FROM information_schema.VIEWS
                      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'statements_with_errors_or_warnings'
                    ),
                    '`performance_schema`.`events_statements_summary_by_digest`',
                    v_digests_table
                );
            IF (@sys.statement_performance_analyzer.limit > 0) THEN
                SET v_sql = CONCAT(v_sql, ' LIMIT ', @sys.statement_performance_analyzer.limit);
            END IF;
            CALL sys.execute_prepared_stmt(v_sql);
        END IF;

        IF (FIND_IN_SET('with_full_table_scans', in_views)) THEN
            SELECT CONCAT('Top ', @sys.statement_performance_analyzer.limit, ' Queries with Full Table Scan') AS 'Next Output';
            SET v_sql =
                REPLACE(
                    (SELECT VIEW_DEFINITION
                       FROM information_schema.VIEWS
                      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'statements_with_full_table_scans'
                    ),
                    '`performance_schema`.`events_statements_summary_by_digest`',
                    v_digests_table
                );
            IF (@sys.statement_performance_analyzer.limit > 0) THEN
                SET v_sql = CONCAT(v_sql, ' LIMIT ', @sys.statement_performance_analyzer.limit);
            END IF;
            CALL sys.execute_prepared_stmt(v_sql);
        END IF;

        IF (FIND_IN_SET('with_sorting', in_views)) THEN
            SELECT CONCAT('Top ', @sys.statement_performance_analyzer.limit, ' Queries with Sorting') AS 'Next Output';
            SET v_sql =
                REPLACE(
                    (SELECT VIEW_DEFINITION
                       FROM information_schema.VIEWS
                      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'statements_with_sorting'
                    ),
                    '`performance_schema`.`events_statements_summary_by_digest`',
                    v_digests_table
                );
            IF (@sys.statement_performance_analyzer.limit > 0) THEN
                SET v_sql = CONCAT(v_sql, ' LIMIT ', @sys.statement_performance_analyzer.limit);
            END IF;
            CALL sys.execute_prepared_stmt(v_sql);
        END IF;

        IF (FIND_IN_SET('with_temp_tables', in_views)) THEN
            SELECT CONCAT('Top ', @sys.statement_performance_analyzer.limit, ' Queries with Internal Temporary Tables') AS 'Next Output';
            SET v_sql =
                REPLACE(
                    (SELECT VIEW_DEFINITION
                       FROM information_schema.VIEWS
                      WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'statements_with_temp_tables'
                    ),
                    '`performance_schema`.`events_statements_summary_by_digest`',
                    v_digests_table
                );
            IF (@sys.statement_performance_analyzer.limit > 0) THEN
                SET v_sql = CONCAT(v_sql, ' LIMIT ', @sys.statement_performance_analyzer.limit);
            END IF;
            CALL sys.execute_prepared_stmt(v_sql);
        END IF;

        IF (FIND_IN_SET('custom', in_views)) THEN
            SELECT CONCAT('Top ', @sys.statement_performance_analyzer.limit, ' Queries Using Custom View') AS 'Next Output';

            IF (@sys.statement_performance_analyzer.view IS NULL) THEN
                SET @sys.statement_performance_analyzer.view = sys.sys_get_config('statement_performance_analyzer.view', NULL);
            END IF;
            IF (@sys.statement_performance_analyzer.view IS NULL) THEN
                SIGNAL SQLSTATE '45000'
                   SET MESSAGE_TEXT = 'The @sys.statement_performance_analyzer.view user variable must be set with the view or query to use.';
            END IF;

            IF (NOT INSTR(@sys.statement_performance_analyzer.view, ' ')) THEN
                -- No spaces, so can't be a query
                IF (NOT INSTR(@sys.statement_performance_analyzer.view, '.')) THEN
                    -- No . in the table name - use current database
                    -- DATABASE() will be the database of the procedure
                    SET v_custom_db   = DATABASE(),
                        v_custom_name = @sys.statement_performance_analyzer.view;
                ELSE
                    SET v_custom_db   = SUBSTRING_INDEX(@sys.statement_performance_analyzer.view, '.', 1);
                    SET v_custom_name = SUBSTRING(@sys.statement_performance_analyzer.view, CHAR_LENGTH(v_custom_db)+2);
                END IF;

                CALL sys.table_exists(v_custom_db, v_custom_name, v_custom_view_exists);
                IF (v_custom_view_exists <> 'VIEW') THEN
                    SIGNAL SQLSTATE '45000'
                       SET MESSAGE_TEXT = 'The @sys.statement_performance_analyzer.view user variable is set but specified neither an existing view nor a query.';
                END IF;

                SET v_sql =
                    REPLACE(
                        (SELECT VIEW_DEFINITION
                           FROM information_schema.VIEWS
                          WHERE TABLE_SCHEMA = v_custom_db AND TABLE_NAME = v_custom_name
                        ),
                        '`performance_schema`.`events_statements_summary_by_digest`',
                        v_digests_table
                    );
            ELSE
                SET v_sql = REPLACE(@sys.statement_performance_analyzer.view, '`performance_schema`.`events_statements_summary_by_digest`', v_digests_table);
            END IF;

            IF (@sys.statement_performance_analyzer.limit > 0) THEN
                SET v_sql = CONCAT(v_sql, ' LIMIT ', @sys.statement_performance_analyzer.limit);
            END IF;

            CALL sys.execute_prepared_stmt(v_sql);
        END IF;
    END IF;

    -- Restore INSTRUMENTED for this thread
    IF (v_this_thread_enabled = 'YES') THEN
        CALL sys.ps_setup_enable_thread(CONNECTION_ID());
    END IF;

    IF (@log_bin = 1) THEN
        SET sql_log_bin = @log_bin;
    END IF;
END$$

DELIMITER ;
