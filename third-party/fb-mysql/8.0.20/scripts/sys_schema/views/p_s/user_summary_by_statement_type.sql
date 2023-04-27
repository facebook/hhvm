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
-- View: user_summary_by_statement_type
--
-- Summarizes the types of statements executed by each user.
-- 
-- When the user found is NULL, it is assumed to be a "background" thread.
--
-- mysql> select * from user_summary_by_statement_type;
-- +------+----------------------+--------+---------------+-------------+--------------+-----------+---------------+---------------+------------+
-- | user | statement            | total  | total_latency | max_latency | lock_latency | rows_sent | rows_examined | rows_affected | full_scans |
-- +------+----------------------+--------+---------------+-------------+--------------+-----------+---------------+---------------+------------+
-- | root | create_view          |   2063 | 00:05:04.20   | 463.58 ms   | 1.42 s       |         0 |             0 |             0 |          0 |
-- | root | select               |    174 | 40.87 s       | 28.83 s     | 858.13 ms    |      5212 |        157022 |             0 |         82 |
-- | root | stmt                 |   6645 | 15.31 s       | 491.78 ms   | 0 ps         |         0 |             0 |          7951 |          0 |
-- | root | call_procedure       |     17 | 4.78 s        | 1.02 s      | 37.94 ms     |         0 |             0 |            19 |          0 |
-- | root | create_table         |     19 | 3.04 s        | 431.71 ms   | 0 ps         |         0 |             0 |             0 |          0 |
-- ...
-- +------+----------------------+--------+---------------+-------------+--------------+-----------+---------------+---------------+------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW user_summary_by_statement_type (
  user,
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
SELECT IF(user IS NULL, 'background', user) AS user,
       SUBSTRING_INDEX(event_name, '/', -1) AS statement,
       count_star AS total,
       format_pico_time(sum_timer_wait) AS total_latency,
       format_pico_time(max_timer_wait) AS max_latency,
       format_pico_time(sum_lock_time) AS lock_latency,
       sum_rows_sent AS rows_sent,
       sum_rows_examined AS rows_examined,
       sum_rows_affected AS rows_affected,
       sum_no_index_used + sum_no_good_index_used AS full_scans
  FROM performance_schema.events_statements_summary_by_user_by_event_name
 WHERE sum_timer_wait != 0
 ORDER BY user, sum_timer_wait DESC;
