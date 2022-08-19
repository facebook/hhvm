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
-- View: waits_by_user_by_latency
--
-- Lists the top wait events per user by their total latency, ignoring idle (this may be very large).
--
-- mysql> select * from waits_by_user_by_latency;
-- +------+-----------------------------------------------------+--------+---------------+-------------+-------------+
-- | user | event                                               | total  | total_latency | avg_latency | max_latency |
-- +------+-----------------------------------------------------+--------+---------------+-------------+-------------+
-- | root | wait/io/file/sql/file_parser                        |  13743 | 00:01:00.46   | 4.40 ms     | 231.88 ms   |
-- | root | wait/io/file/innodb/innodb_data_file                |   4699 | 3.02 s        | 643.38 us   | 46.93 ms    |
-- | root | wait/io/file/sql/FRM                                |  11462 | 2.60 s        | 226.83 us   | 61.72 ms    |
-- | root | wait/io/file/myisam/dfile                           |  26776 | 746.70 ms     | 27.89 us    | 308.79 ms   |
-- | root | wait/io/file/myisam/kfile                           |   7126 | 462.66 ms     | 64.93 us    | 88.76 ms    |
-- | root | wait/io/file/sql/dbopt                              |    179 | 137.58 ms     | 768.59 us   | 15.46 ms    |
-- | root | wait/io/file/csv/metadata                           |      8 | 86.60 ms      | 10.82 ms    | 50.32 ms    |
-- | root | wait/synch/mutex/mysys/IO_CACHE::append_buffer_lock | 798080 | 66.46 ms      | 82.94 ns    | 161.03 us   |
-- | root | wait/io/file/sql/binlog                             |     19 | 49.11 ms      | 2.58 ms     | 9.40 ms     |
-- | root | wait/io/file/sql/misc                               |     26 | 22.38 ms      | 860.80 us   | 15.30 ms    |
-- | root | wait/io/file/csv/data                               |      4 | 297.46 us     | 74.37 us    | 111.93 us   |
-- | root | wait/synch/rwlock/sql/MDL_lock::rwlock              |    944 | 287.86 us     | 304.62 ns   | 874.64 ns   |
-- | root | wait/io/file/archive/data                           |      4 | 82.71 us      | 20.68 us    | 40.74 us    |
-- | root | wait/synch/mutex/myisam/MYISAM_SHARE::intern_lock   |     60 | 12.21 us      | 203.20 ns   | 512.72 ns   |
-- | root | wait/synch/mutex/innodb/trx_mutex                   |     81 | 5.93 us       | 73.14 ns    | 252.59 ns   |
-- +------+-----------------------------------------------------+--------+---------------+-------------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW waits_by_user_by_latency (
  user,
  event,
  total,
  total_latency,
  avg_latency,
  max_latency
) AS
SELECT IF(user IS NULL, 'background', user) AS user,
       event_name AS event,
       count_star AS total,
       format_pico_time(sum_timer_wait) AS total_latency,
       format_pico_time(avg_timer_wait) AS avg_latency,
       format_pico_time(max_timer_wait) AS max_latency
  FROM performance_schema.events_waits_summary_by_user_by_event_name
 WHERE event_name != 'idle'
   AND user IS NOT NULL
   AND sum_timer_wait > 0
 ORDER BY user, sum_timer_wait DESC;
