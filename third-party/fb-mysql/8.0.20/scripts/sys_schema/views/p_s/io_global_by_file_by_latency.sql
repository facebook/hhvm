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
-- View: io_global_by_file_by_latency
--
-- Shows the top global IO consumers by latency by file.
--
-- mysql> select * from io_global_by_file_by_latency limit 5;
-- +-----------------------------------------------------------+-------+---------------+------------+--------------+-------------+---------------+------------+--------------+
-- | file                                                      | total | total_latency | count_read | read_latency | count_write | write_latency | count_misc | misc_latency |
-- +-----------------------------------------------------------+-------+---------------+------------+--------------+-------------+---------------+------------+--------------+
-- | @@datadir/sys/wait_classes_global_by_avg_latency_raw.frm~ |    24 | 451.99 ms     |          0 | 0 ps         |           4 | 108.07 us     |         20 | 451.88 ms    |
-- | @@datadir/sys/innodb_buffer_stats_by_schema_raw.frm~      |    24 | 379.84 ms     |          0 | 0 ps         |           4 | 108.88 us     |         20 | 379.73 ms    |
-- | @@datadir/sys/io_by_thread_by_latency_raw.frm~            |    24 | 379.46 ms     |          0 | 0 ps         |           4 | 101.37 us     |         20 | 379.36 ms    |
-- | @@datadir/ibtmp1                                          |    53 | 373.45 ms     |          0 | 0 ps         |          48 | 246.08 ms     |          5 | 127.37 ms    |
-- | @@datadir/sys/statement_analysis_raw.frm~                 |    24 | 353.14 ms     |          0 | 0 ps         |           4 | 94.96 us      |         20 | 353.04 ms    |
-- +-----------------------------------------------------------+-------+---------------+------------+--------------+-------------+---------------+------------+--------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW io_global_by_file_by_latency (
  file,
  total,
  total_latency,
  count_read,
  read_latency,
  count_write,
  write_latency,
  count_misc,
  misc_latency
) AS
SELECT sys.format_path(file_name) AS file, 
       count_star AS total, 
       format_pico_time(sum_timer_wait) AS total_latency,
       count_read,
       format_pico_time(sum_timer_read) AS read_latency,
       count_write,
       format_pico_time(sum_timer_write) AS write_latency,
       count_misc,
       format_pico_time(sum_timer_misc) AS misc_latency
  FROM performance_schema.file_summary_by_instance
 ORDER BY sum_timer_wait DESC;
