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
-- View: wait_classes_global_by_latency
-- 
-- Lists the top wait classes by total latency, ignoring idle (this may be very large).
--
-- mysql> select * from wait_classes_global_by_latency;
-- +-------------------+--------+---------------+-------------+-------------+-------------+
-- | event_class       | total  | total_latency | min_latency | avg_latency | max_latency |
-- +-------------------+--------+---------------+-------------+-------------+-------------+
-- | wait/io/file      | 550470 | 46.01 s       | 19.44 ns    | 83.58 us    | 4.21 s      |
-- | wait/io/socket    | 228833 | 2.71 s        | 0 ps        | 11.86 us    | 29.93 ms    |
-- | wait/io/table     |  64063 | 1.89 s        | 99.79 ns    | 29.43 us    | 68.07 ms    |
-- | wait/lock/table   |  76029 | 47.19 ms      | 65.45 ns    | 620.74 ns   | 969.88 us   |
-- | wait/synch/mutex  | 635925 | 34.93 ms      | 19.44 ns    | 54.93 ns    | 107.70 us   |
-- | wait/synch/rwlock |  61287 | 7.62 ms       | 21.38 ns    | 124.37 ns   | 34.65 us    |
-- +-------------------+--------+---------------+-------------+-------------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW wait_classes_global_by_latency (
  event_class,
  total,
  total_latency,
  min_latency,
  avg_latency,
  max_latency
) AS
SELECT SUBSTRING_INDEX(event_name,'/', 3) AS event_class, 
       SUM(COUNT_STAR) AS total,
       format_pico_time(SUM(sum_timer_wait)) AS total_latency,
       format_pico_time(MIN(min_timer_wait)) min_latency,
       format_pico_time(IFNULL(SUM(sum_timer_wait) / NULLIF(SUM(COUNT_STAR), 0), 0)) AS avg_latency,
       format_pico_time(MAX(max_timer_wait)) AS max_latency
  FROM performance_schema.events_waits_summary_global_by_event_name
 WHERE sum_timer_wait > 0
   AND event_name != 'idle'
 GROUP BY SUBSTRING_INDEX(event_name,'/', 3) 
 ORDER BY SUM(sum_timer_wait) DESC;
