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
-- View: io_global_by_wait_by_bytes
--
-- Shows the top global IO consumer classes by bytes usage.
--
-- mysql> select * from io_global_by_wait_by_bytes;
-- +--------------------+--------+---------------+-------------+-------------+-------------+------------+------------+-----------+-------------+---------------+-------------+-----------------+
-- | event_name         | total  | total_latency | min_latency | avg_latency | max_latency | count_read | total_read | avg_read  | count_write | total_written | avg_written | total_requested |
-- +--------------------+--------+---------------+-------------+-------------+-------------+------------+------------+-----------+-------------+---------------+-------------+-----------------+
-- | myisam/dfile       | 163681 | 983.13 ms     | 379.08 ns   | 6.01 us     | 22.06 ms    |      68737 | 127.31 MiB | 1.90 KiB  |     1012221 | 121.52 MiB    | 126 bytes   | 248.83 MiB      |
-- | myisam/kfile       |   1775 | 375.13 ms     | 1.02 us     | 211.34 us   | 35.15 ms    |      54066 | 9.97 MiB   | 193 bytes |      428257 | 12.40 MiB     | 30 bytes    | 22.37 MiB       |
-- | sql/FRM            |  57889 | 8.40 s        | 19.44 ns    | 145.05 us   | 336.71 ms   |       8009 | 2.60 MiB   | 341 bytes |       14675 | 2.91 MiB      | 208 bytes   | 5.51 MiB        |
-- | sql/global_ddl_log |    164 | 75.96 ms      | 5.72 us     | 463.19 us   | 7.43 ms     |         20 | 80.00 KiB  | 4.00 KiB  |          76 | 304.00 KiB    | 4.00 KiB    | 384.00 KiB      |
-- | sql/file_parser    |    419 | 601.37 ms     | 1.96 us     | 1.44 ms     | 37.14 ms    |         66 | 42.01 KiB  | 652 bytes |          64 | 226.98 KiB    | 3.55 KiB    | 268.99 KiB      |
-- | sql/binlog         |    190 | 6.79 s        | 1.56 us     | 35.76 ms    | 4.21 s      |         52 | 60.54 KiB  | 1.16 KiB  |           0 | 0 bytes       | 0 bytes     | 60.54 KiB       |
-- | sql/ERRMSG         |      5 | 2.03 s        | 8.61 us     | 405.40 ms   | 2.03 s      |          3 | 51.82 KiB  | 17.27 KiB |           0 | 0 bytes       | 0 bytes     | 51.82 KiB       |
-- | mysys/charset      |      3 | 196.52 us     | 17.61 us    | 65.51 us    | 137.33 us   |          1 | 17.83 KiB  | 17.83 KiB |           0 | 0 bytes       | 0 bytes     | 17.83 KiB       |
-- | sql/partition      |     81 | 18.87 ms      | 888.08 ns   | 232.92 us   | 4.67 ms     |         66 | 2.75 KiB   | 43 bytes  |           8 | 288 bytes     | 36 bytes    | 3.04 KiB        |
-- | sql/dbopt          | 329166 | 26.95 s       | 2.06 us     | 81.89 us    | 178.71 ms   |          0 | 0 bytes    | 0 bytes   |           9 | 585 bytes     | 65 bytes    | 585 bytes       |
-- | sql/relaylog       |      7 | 1.18 ms       | 838.84 ns   | 168.30 us   | 892.70 us   |          0 | 0 bytes    | 0 bytes   |           1 | 120 bytes     | 120 bytes   | 120 bytes       |
-- | mysys/cnf          |      5 | 171.61 us     | 303.26 ns   | 34.32 us    | 115.21 us   |          3 | 56 bytes   | 19 bytes  |           0 | 0 bytes       | 0 bytes     | 56 bytes        |
-- | sql/pid            |      3 | 220.55 us     | 29.29 us    | 73.52 us    | 143.11 us   |          0 | 0 bytes    | 0 bytes   |           1 | 5 bytes       | 5 bytes     | 5 bytes         |
-- | sql/casetest       |      1 | 121.19 us     | 121.19 us   | 121.19 us   | 121.19 us   |          0 | 0 bytes    | 0 bytes   |           0 | 0 bytes       | 0 bytes     | 0 bytes         |
-- | sql/binlog_index   |      5 | 593.47 us     | 1.07 us     | 118.69 us   | 535.90 us   |          0 | 0 bytes    | 0 bytes   |           0 | 0 bytes       | 0 bytes     | 0 bytes         |
-- | sql/misc           |     23 | 2.73 ms       | 65.14 us    | 118.50 us   | 255.31 us   |          0 | 0 bytes    | 0 bytes   |           0 | 0 bytes       | 0 bytes     | 0 bytes         |
-- +--------------------+--------+---------------+-------------+-------------+-------------+------------+------------+-----------+-------------+---------------+-------------+-----------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW io_global_by_wait_by_bytes (
  event_name,
  total,
  total_latency,
  min_latency,
  avg_latency,
  max_latency,
  count_read,
  total_read,
  avg_read,
  count_write,
  total_written,
  avg_written,
  total_requested
) AS
SELECT SUBSTRING_INDEX(event_name, '/', -2) event_name,
       count_star AS total,
       format_pico_time(sum_timer_wait) AS total_latency,
       format_pico_time(min_timer_wait) AS min_latency,
       format_pico_time(avg_timer_wait) AS avg_latency,
       format_pico_time(max_timer_wait) AS max_latency,
       count_read,
       format_bytes(sum_number_of_bytes_read) AS total_read,
       format_bytes(IFNULL(sum_number_of_bytes_read / NULLIF(count_read, 0), 0)) AS avg_read,
       count_write,
       format_bytes(sum_number_of_bytes_write) AS total_written,
       format_bytes(IFNULL(sum_number_of_bytes_write / NULLIF(count_write, 0), 0)) AS avg_written,
       format_bytes(sum_number_of_bytes_write + sum_number_of_bytes_read) AS total_requested
  FROM performance_schema.file_summary_by_event_name
 WHERE event_name LIKE 'wait/io/file/%' 
   AND count_star > 0
 ORDER BY sum_number_of_bytes_write + sum_number_of_bytes_read DESC;
