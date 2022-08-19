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
-- View: wait_classes_global_by_avg_latency
-- 
-- Lists the top wait classes by average latency, ignoring idle (this may be very large).
--
-- mysql> select * from wait_classes_global_by_avg_latency where event_class != 'idle';
-- +-------------------+--------+---------------+-------------+-------------+-------------+
-- | event_class       | total  | total_latency | min_latency | avg_latency | max_latency |
-- +-------------------+--------+---------------+-------------+-------------+-------------+
-- | wait/io/file      | 543123 | 44.60 s       | 19.44 ns    | 82.11 us    | 4.21 s      |
-- | wait/io/table     |  22002 | 766.60 ms     | 148.72 ns   | 34.84 us    | 44.97 ms    |
-- | wait/io/socket    |  79613 | 967.17 ms     | 0 ps        | 12.15 us    | 27.10 ms    |
-- | wait/lock/table   |  35409 | 18.68 ms      | 65.45 ns    | 527.51 ns   | 969.88 us   |
-- | wait/synch/rwlock |  37935 | 4.61 ms       | 21.38 ns    | 121.61 ns   | 34.65 us    |
-- | wait/synch/mutex  | 390622 | 18.60 ms      | 19.44 ns    | 47.61 ns    | 10.32 us    |
-- +-------------------+--------+---------------+-------------+-------------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW wait_classes_global_by_avg_latency (
  event_class,
  total,
  total_latency,
  min_latency,
  avg_latency,
  max_latency
) AS
SELECT SUBSTRING_INDEX(event_name,'/', 3) AS event_class,
       SUM(COUNT_STAR) AS total,
       format_pico_time(CAST(SUM(sum_timer_wait) AS UNSIGNED)) AS total_latency,
       format_pico_time(MIN(min_timer_wait)) AS min_latency,
       format_pico_time(IFNULL(SUM(sum_timer_wait) / NULLIF(SUM(COUNT_STAR), 0), 0)) AS avg_latency,
       format_pico_time(CAST(MAX(max_timer_wait) AS UNSIGNED)) AS max_latency
  FROM performance_schema.events_waits_summary_global_by_event_name
 WHERE sum_timer_wait > 0
   AND event_name != 'idle'
 GROUP BY event_class
 ORDER BY IFNULL(SUM(sum_timer_wait) / NULLIF(SUM(COUNT_STAR), 0), 0) DESC;
