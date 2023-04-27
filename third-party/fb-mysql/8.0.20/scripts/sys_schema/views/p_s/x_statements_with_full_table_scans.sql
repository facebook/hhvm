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
-- View: x$statements_with_full_table_scans
--
-- Lists all normalized statements that use have done a full table scan
-- ordered by number the percentage of times a full scan was done,
-- then by the statement latency.
--
-- This view ignores SHOW statements, as these always cause a full table scan,
-- and there is nothing that can be done about this.
--
-- mysql> select * from x$statements_with_full_table_scans limit 1\G
-- *************************** 1. row ***************************
--                    query: SELECT * FROM `schema_object_overview` SELECT `information_schema` . `routines` . `ROUTINE_SCHEMA` // truncated
--                       db: sys
--               exec_count: 2
--            total_latency: 16751388791000
--      no_index_used_count: 2
-- no_good_index_used_count: 0
--        no_index_used_pct: 100
--                rows_sent: 84
--            rows_examined: 20012
--            rows_sent_avg: 42
--        rows_examined_avg: 10006
--               first_seen: 2014-03-07 13:13:41
--                last_seen: 2014-03-07 13:13:48
--                   digest: 54f9bd520f0bbf15db0c2ed93386bec9
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$statements_with_full_table_scans (
  query,
  db,
  exec_count,
  total_latency,
  no_index_used_count,
  no_good_index_used_count,
  no_index_used_pct,
  rows_sent,
  rows_examined,
  rows_sent_avg,
  rows_examined_avg,
  first_seen,
  last_seen,
  digest
) AS
SELECT DIGEST_TEXT AS query,
       SCHEMA_NAME as db,
       COUNT_STAR AS exec_count,
       SUM_TIMER_WAIT AS total_latency,
       SUM_NO_INDEX_USED AS no_index_used_count,
       SUM_NO_GOOD_INDEX_USED AS no_good_index_used_count,
       ROUND(IFNULL(SUM_NO_INDEX_USED / NULLIF(COUNT_STAR, 0), 0) * 100) AS no_index_used_pct,
       SUM_ROWS_SENT AS rows_sent,
       SUM_ROWS_EXAMINED AS rows_examined,
       ROUND(SUM_ROWS_SENT/COUNT_STAR) AS rows_sent_avg,
       ROUND(SUM_ROWS_EXAMINED/COUNT_STAR) AS rows_examined_avg,
       FIRST_SEEN as first_seen,
       LAST_SEEN as last_seen,
       DIGEST AS digest
  FROM performance_schema.events_statements_summary_by_digest
 WHERE (SUM_NO_INDEX_USED > 0
    OR SUM_NO_GOOD_INDEX_USED > 0)
   AND DIGEST_TEXT NOT LIKE 'SHOW%'
 ORDER BY no_index_used_pct DESC, total_latency DESC;
