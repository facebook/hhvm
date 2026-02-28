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
-- View: host_summary_by_statement_latency
--
-- Summarizes overall statement statistics by host.
--
-- When the host found is NULL, it is assumed to be a "background" thread.
--
-- mysql> select-- from host_summary_by_statement_latency;
-- +------+-------+---------------+-------------+--------------+-----------+---------------+---------------+------------+
-- | host | total | total_latency | max_latency | lock_latency | rows_sent | rows_examined | rows_affected | full_scans |
-- +------+-------+---------------+-------------+--------------+-----------+---------------+---------------+------------+
-- | hal  |  3381 | 00:02:09.13   | 1.48 s      | 1.07 s       |      1151 |         93947 |           150 |         91 |
-- +------+-------+---------------+-------------+--------------+-----------+---------------+---------------+------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW host_summary_by_statement_latency (
  host,
  total,
  total_latency,
  max_latency,
  lock_latency,
  rows_sent,
  rows_examined,
  rows_affected,
  full_scans
) AS
SELECT IF(host IS NULL, 'background', host) AS host,
       SUM(count_star) AS total,
       format_pico_time(SUM(sum_timer_wait)) AS total_latency,
       format_pico_time(MAX(max_timer_wait)) AS max_latency,
       format_pico_time(SUM(sum_lock_time)) AS lock_latency,
       SUM(sum_rows_sent) AS rows_sent,
       SUM(sum_rows_examined) AS rows_examined,
       SUM(sum_rows_affected) AS rows_affected,
       SUM(sum_no_index_used) + SUM(sum_no_good_index_used) AS full_scans
  FROM performance_schema.events_statements_summary_by_host_by_event_name
 GROUP BY IF(host IS NULL, 'background', host)
 ORDER BY SUM(sum_timer_wait) DESC;
