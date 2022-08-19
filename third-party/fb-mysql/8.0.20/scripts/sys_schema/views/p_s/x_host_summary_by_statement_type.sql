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
-- View: x$host_summary_by_statement_type
--
-- Summarizes the types of statements executed by each host.
--
-- When the host found is NULL, it is assumed to be a "background" thread.
--
-- mysql> select * from x$host_summary_by_statement_type;
-- +------+----------------------+--------+-----------------+----------------+----------------+-----------+---------------+---------------+------------+
-- | host | statement            | total  | total_latency   | max_latency    | lock_latency   | rows_sent | rows_examined | rows_affected | full_scans |
-- +------+----------------------+--------+-----------------+----------------+----------------+-----------+---------------+---------------+------------+
-- | hal  | create_view          |   2110 | 312717366332000 |   463578029000 |  1432355000000 |         0 |             0 |             0 |          0 |
-- | hal  | select               |    177 |  41115690428000 | 28827579292000 |   858709000000 |      5254 |        157437 |             0 |         83 |
-- | hal  | stmt                 |   6645 |  15305389969000 |   491780297000 |              0 |         0 |             0 |          7951 |          0 |
-- | hal  | call_procedure       |     17 |   4783806053000 |  1016083397000 |    37936000000 |         0 |             0 |            19 |          0 |
-- | hal  | create_table         |     19 |   3035120946000 |   431706815000 |              0 |         0 |             0 |             0 |          0 |
-- ...
-- +------+----------------------+--------+-----------------+----------------+----------------+-----------+---------------+---------------+------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$host_summary_by_statement_type (
  host,
  statement,
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
       SUBSTRING_INDEX(event_name, '/', -1) AS statement,
       count_star AS total,
       sum_timer_wait AS total_latency,
       max_timer_wait AS max_latency,
       sum_lock_time AS lock_latency,
       sum_rows_sent AS rows_sent,
       sum_rows_examined AS rows_examined,
       sum_rows_affected AS rows_affected,
       sum_no_index_used + sum_no_good_index_used AS full_scans
  FROM performance_schema.events_statements_summary_by_host_by_event_name
 WHERE sum_timer_wait != 0
 ORDER BY IF(host IS NULL, 'background', host), sum_timer_wait DESC;
