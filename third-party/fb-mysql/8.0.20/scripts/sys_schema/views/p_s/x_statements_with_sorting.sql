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
-- View: x$statements_with_sorting
--
-- Lists all normalized statements that have done sorts,
-- ordered by total_latency descending.
--
-- mysql> select * from x$statements_with_sorting\G
-- *************************** 1. row ***************************
--             query: SELECT * FROM `schema_object_overview` SELECT `information_schema` . `routines` . `ROUTINE_SCHEMA` AS ... truncated
--                db: sys
--        exec_count: 2
--     total_latency: 16751388791000
-- sort_merge_passes: 0
--   avg_sort_merges: 0
-- sorts_using_scans: 12
--  sort_using_range: 0
--       rows_sorted: 168
--   avg_rows_sorted: 84
--        first_seen: 2014-03-07 13:13:41
--         last_seen: 2014-03-07 13:13:48
--            digest: 54f9bd520f0bbf15db0c2ed93386bec9
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$statements_with_sorting (
  query,
  db,
  exec_count,
  total_latency,
  sort_merge_passes,
  avg_sort_merges,
  sorts_using_scans,
  sort_using_range,
  rows_sorted,
  avg_rows_sorted,
  first_seen,
  last_seen,
  digest
) AS
SELECT DIGEST_TEXT AS query,
       SCHEMA_NAME db,
       COUNT_STAR AS exec_count,
       SUM_TIMER_WAIT AS total_latency,
       SUM_SORT_MERGE_PASSES AS sort_merge_passes,
       ROUND(IFNULL(SUM_SORT_MERGE_PASSES / NULLIF(COUNT_STAR, 0), 0)) AS avg_sort_merges,
       SUM_SORT_SCAN AS sorts_using_scans,
       SUM_SORT_RANGE AS sort_using_range,
       SUM_SORT_ROWS AS rows_sorted,
       ROUND(IFNULL(SUM_SORT_ROWS / NULLIF(COUNT_STAR, 0), 0)) AS avg_rows_sorted,
       FIRST_SEEN as first_seen,
       LAST_SEEN as last_seen,
       DIGEST AS digest
  FROM performance_schema.events_statements_summary_by_digest
 WHERE SUM_SORT_ROWS > 0
 ORDER BY SUM_TIMER_WAIT DESC;
