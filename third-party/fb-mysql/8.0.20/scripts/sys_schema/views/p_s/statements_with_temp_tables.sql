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

--
-- View: statements_with_temp_tables
--
-- Lists all normalized statements that use temporary tables
-- ordered by number of on disk temporary tables descending first, 
-- then by the number of memory tables.
--
-- mysql> select * from statements_with_temp_tables limit 1\G
-- *************************** 1. row ***************************
--                    query: SELECT * FROM `schema_object_o ... MA` , `information_schema` ...
--                       db: sys
--               exec_count: 2
--            total_latency: 16.75 s
--        memory_tmp_tables: 378
--          disk_tmp_tables: 66
-- avg_tmp_tables_per_query: 189
--   tmp_tables_to_disk_pct: 17
--               first_seen: 2014-03-07 13:13:41
--                last_seen: 2014-03-07 13:13:48
--                   digest: 54f9bd520f0bbf15db0c2ed93386bec9
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW statements_with_temp_tables (
  query,
  db,
  exec_count,
  total_latency,
  memory_tmp_tables,
  disk_tmp_tables,
  avg_tmp_tables_per_query,
  tmp_tables_to_disk_pct,
  first_seen,
  last_seen,
  digest
) AS
SELECT sys.format_statement(DIGEST_TEXT) AS query,
       SCHEMA_NAME as db,
       COUNT_STAR AS exec_count,
       format_pico_time(SUM_TIMER_WAIT) as total_latency,
       SUM_CREATED_TMP_TABLES AS memory_tmp_tables,
       SUM_CREATED_TMP_DISK_TABLES AS disk_tmp_tables,
       ROUND(IFNULL(SUM_CREATED_TMP_TABLES / NULLIF(COUNT_STAR, 0), 0)) AS avg_tmp_tables_per_query,
       ROUND(IFNULL(SUM_CREATED_TMP_DISK_TABLES / NULLIF(SUM_CREATED_TMP_TABLES, 0), 0) * 100) AS tmp_tables_to_disk_pct,
       FIRST_SEEN as first_seen,
       LAST_SEEN as last_seen,
       DIGEST AS digest
  FROM performance_schema.events_statements_summary_by_digest
 WHERE SUM_CREATED_TMP_TABLES > 0
ORDER BY SUM_CREATED_TMP_DISK_TABLES DESC, SUM_CREATED_TMP_TABLES DESC;
