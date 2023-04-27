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
-- View: x$statements_with_runtimes_in_95th_percentile
--
-- List all statements whose average runtime, in microseconds, is in the top 95th percentile.
-- 
-- mysql> SELECT * FROM x$statements_with_runtimes_in_95th_percentile LIMIT 1\G
-- *************************** 1. row ***************************
--             query: SELECT `e` . `round_robin_bin` AS `round1_1706_0_` , `e` . `id` AS `id1706_0_` , `e` . `timestamp` AS `timestamp1706_0_` , ... truncated
--                db: mem
--         full_scan: *
--        exec_count: 14
--         err_count: 0
--        warn_count: 0
--     total_latency: 43961670267000
--       max_latency: 6686877140000
--       avg_latency: 3140119304000
--         rows_sent: 11
--     rows_sent_avg: 1
--     rows_examined: 253170
-- rows_examined_avg: 18084
--        first_seen: 2013-12-04 20:05:01
--         last_seen: 2013-12-04 20:06:34
--            digest: 29ba002bf039bb6439357a10134407de
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$statements_with_runtimes_in_95th_percentile (
  query,
  db,
  full_scan,
  exec_count,
  err_count,
  warn_count,
  total_latency,
  max_latency,
  avg_latency,
  rows_sent,
  rows_sent_avg,
  rows_examined,
  rows_examined_avg,
  first_seen,
  last_seen,
  digest
) AS
SELECT DIGEST_TEXT AS query,
       SCHEMA_NAME AS db,
       IF(SUM_NO_GOOD_INDEX_USED > 0 OR SUM_NO_INDEX_USED > 0, '*', '') AS full_scan,
       COUNT_STAR AS exec_count,
       SUM_ERRORS AS err_count,
       SUM_WARNINGS AS warn_count,
       SUM_TIMER_WAIT AS total_latency,
       MAX_TIMER_WAIT AS max_latency,
       AVG_TIMER_WAIT AS avg_latency,
       SUM_ROWS_SENT AS rows_sent,
       ROUND(IFNULL(SUM_ROWS_SENT / NULLIF(COUNT_STAR, 0), 0)) AS rows_sent_avg,
       SUM_ROWS_EXAMINED AS rows_examined,
       ROUND(IFNULL(SUM_ROWS_EXAMINED / NULLIF(COUNT_STAR, 0), 0)) AS rows_examined_avg,
       FIRST_SEEN as first_seen,
       LAST_SEEN as last_seen,
       DIGEST AS digest
  FROM performance_schema.events_statements_summary_by_digest stmts
  JOIN sys.x$ps_digest_95th_percentile_by_avg_us AS top_percentile
    ON ROUND(stmts.avg_timer_wait/1000000) >= top_percentile.avg_us
 ORDER BY AVG_TIMER_WAIT DESC;
