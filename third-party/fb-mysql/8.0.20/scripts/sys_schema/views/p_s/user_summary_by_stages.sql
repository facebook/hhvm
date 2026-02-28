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
-- View: user_summary_by_stages
--
-- Summarizes stages by user, ordered by user and total latency per stage.
-- 
-- When the user found is NULL, it is assumed to be a "background" thread.  
-- 
-- mysql> select * from user_summary_by_stages;
-- +------+--------------------------------+-------+---------------+-------------+
-- | user | event_name                     | total | total_latency | avg_latency |
-- +------+--------------------------------+-------+---------------+-------------+
-- | root | stage/sql/Opening tables       |   889 | 1.97 ms       | 2.22 us     |
-- | root | stage/sql/Creating sort index  |     4 | 1.79 ms       | 446.30 us   |
-- | root | stage/sql/init                 |    10 | 312.27 us     | 31.23 us    |
-- | root | stage/sql/checking permissions |    10 | 300.62 us     | 30.06 us    |
-- | root | stage/sql/freeing items        |     5 | 85.89 us      | 17.18 us    |
-- | root | stage/sql/statistics           |     5 | 79.15 us      | 15.83 us    |
-- | root | stage/sql/preparing            |     5 | 69.12 us      | 13.82 us    |
-- | root | stage/sql/optimizing           |     5 | 53.11 us      | 10.62 us    |
-- | root | stage/sql/Sending data         |     5 | 44.66 us      | 8.93 us     |
-- | root | stage/sql/closing tables       |     5 | 37.54 us      | 7.51 us     |
-- | root | stage/sql/System lock          |     5 | 34.28 us      | 6.86 us     |
-- | root | stage/sql/query end            |     5 | 24.37 us      | 4.87 us     |
-- | root | stage/sql/end                  |     5 | 8.60 us       | 1.72 us     |
-- | root | stage/sql/Sorting result       |     5 | 8.33 us       | 1.67 us     |
-- | root | stage/sql/executing            |     5 | 5.37 us       | 1.07 us     |
-- | root | stage/sql/cleaning up          |     5 | 4.60 us       | 919.00 ns   |
-- +------+--------------------------------+-------+---------------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW user_summary_by_stages (
  user,
  event_name,
  total,
  total_latency,
  avg_latency
) AS
SELECT IF(user IS NULL, 'background', user) AS user,
       event_name,
       count_star AS total,
       format_pico_time(sum_timer_wait) AS total_latency, 
       format_pico_time(avg_timer_wait) AS avg_latency 
  FROM performance_schema.events_stages_summary_by_user_by_event_name
 WHERE sum_timer_wait != 0 
 ORDER BY user, sum_timer_wait DESC;
