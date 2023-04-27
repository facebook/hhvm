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
-- View: io_global_by_wait_by_latency
--
-- Shows the top global IO consumers by latency.
--
-- mysql> SELECT * FROM io_global_by_wait_by_latency;
-- +-------------------------+-------+---------------+-------------+-------------+--------------+---------------+--------------+------------+------------+-----------+-------------+---------------+-------------+
-- | event_name              | total | total_latency | avg_latency | max_latency | read_latency | write_latency | misc_latency | count_read | total_read | avg_read  | count_write | total_written | avg_written |
-- +-------------------------+-------+---------------+-------------+-------------+--------------+---------------+--------------+------------+------------+-----------+-------------+---------------+-------------+
-- | sql/file_parser         |  5433 | 30.20 s       | 5.56 ms     | 203.65 ms   | 22.08 ms     | 24.89 ms      | 30.16 s      |         24 | 6.18 KiB   | 264 bytes |         737 | 2.15 MiB      | 2.99 KiB    |
-- | innodb/innodb_data_file |  1344 | 1.52 s        | 1.13 ms     | 350.70 ms   | 203.82 ms    | 450.96 ms     | 868.21 ms    |        147 | 2.30 MiB   | 16.00 KiB |        1001 | 53.61 MiB     | 54.84 KiB   |
-- | innodb/innodb_log_file  |   828 | 893.48 ms     | 1.08 ms     | 30.11 ms    | 16.32 ms     | 705.89 ms     | 171.27 ms    |          6 | 68.00 KiB  | 11.33 KiB |         413 | 2.19 MiB      | 5.42 KiB    |
-- | myisam/kfile            |  7642 | 242.34 ms     | 31.71 us    | 19.27 ms    | 73.60 ms     | 23.48 ms      | 145.26 ms    |        758 | 135.63 KiB | 183 bytes |        4386 | 232.52 KiB    | 54 bytes    |
-- | myisam/dfile            | 12540 | 223.47 ms     | 17.82 us    | 32.50 ms    | 87.76 ms     | 16.97 ms      | 118.74 ms    |       5390 | 4.49 MiB   | 873 bytes |        1448 | 2.65 MiB      | 1.88 KiB    |
-- | csv/metadata            |     8 | 28.98 ms      | 3.62 ms     | 20.15 ms    | 399.27 us    | 0 ps          | 28.58 ms     |          2 | 70 bytes   | 35 bytes  |           0 | 0 bytes       | 0 bytes     |
-- | mysys/charset           |     3 | 24.24 ms      | 8.08 ms     | 24.15 ms    | 24.15 ms     | 0 ps          | 93.18 us     |          1 | 17.31 KiB  | 17.31 KiB |           0 | 0 bytes       | 0 bytes     |
-- | sql/ERRMSG              |     5 | 20.43 ms      | 4.09 ms     | 19.31 ms    | 20.32 ms     | 0 ps          | 103.20 us    |          3 | 58.97 KiB  | 19.66 KiB |           0 | 0 bytes       | 0 bytes     |
-- | mysys/cnf               |     5 | 11.37 ms      | 2.27 ms     | 11.28 ms    | 11.29 ms     | 0 ps          | 78.22 us     |          3 | 56 bytes   | 19 bytes  |           0 | 0 bytes       | 0 bytes     |
-- | sql/dbopt               |    57 | 4.04 ms       | 70.92 us    | 843.70 us   | 0 ps         | 186.43 us     | 3.86 ms      |          0 | 0 bytes    | 0 bytes   |           7 | 431 bytes     | 62 bytes    |
-- | csv/data                |     4 | 411.55 us     | 102.89 us   | 234.89 us   | 0 ps         | 0 ps          | 411.55 us    |          0 | 0 bytes    | 0 bytes   |           0 | 0 bytes       | 0 bytes     |
-- | sql/misc                |    22 | 340.38 us     | 15.47 us    | 33.77 us    | 0 ps         | 0 ps          | 340.38 us    |          0 | 0 bytes    | 0 bytes   |           0 | 0 bytes       | 0 bytes     |
-- | archive/data            |    39 | 277.86 us     | 7.12 us     | 16.18 us    | 0 ps         | 0 ps          | 277.86 us    |          0 | 0 bytes    | 0 bytes   |           0 | 0 bytes       | 0 bytes     |
-- | sql/pid                 |     3 | 218.03 us     | 72.68 us    | 154.84 us   | 0 ps         | 21.64 us      | 196.39 us    |          0 | 0 bytes    | 0 bytes   |           1 | 6 bytes       | 6 bytes     |
-- | sql/casetest            |     5 | 197.15 us     | 39.43 us    | 126.31 us   | 0 ps         | 0 ps          | 197.15 us    |          0 | 0 bytes    | 0 bytes   |           0 | 0 bytes       | 0 bytes     |
-- | sql/global_ddl_log      |     2 | 14.60 us      | 7.30 us     | 12.12 us    | 0 ps         | 0 ps          | 14.60 us     |          0 | 0 bytes    | 0 bytes   |           0 | 0 bytes       | 0 bytes     |
-- +-------------------------+-------+---------------+-------------+-------------+--------------+---------------+--------------+------------+------------+-----------+-------------+---------------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW io_global_by_wait_by_latency (
  event_name,
  total,
  total_latency,
  avg_latency,
  max_latency,
  read_latency,
  write_latency,
  misc_latency,
  count_read,
  total_read,
  avg_read,
  count_write,
  total_written,
  avg_written
) AS
SELECT SUBSTRING_INDEX(event_name, '/', -2) AS event_name,
       count_star AS total,
       format_pico_time(sum_timer_wait) AS total_latency,
       format_pico_time(avg_timer_wait) AS avg_latency,
       format_pico_time(max_timer_wait) AS max_latency,
       format_pico_time(sum_timer_read) AS read_latency,
       format_pico_time(sum_timer_write) AS write_latency,
       format_pico_time(sum_timer_misc) AS misc_latency,
       count_read,
       format_bytes(sum_number_of_bytes_read) AS total_read,
       format_bytes(IFNULL(sum_number_of_bytes_read / NULLIF(count_read, 0), 0)) AS avg_read,
       count_write,
       format_bytes(sum_number_of_bytes_write) AS total_written,
       format_bytes(IFNULL(sum_number_of_bytes_write / NULLIF(count_write, 0), 0)) AS avg_written
  FROM performance_schema.file_summary_by_event_name 
 WHERE event_name LIKE 'wait/io/file/%'
   AND count_star > 0
 ORDER BY sum_timer_wait DESC;
