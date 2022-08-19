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
-- View: x$io_global_by_wait_by_bytes
--
-- Shows the top global IO consumer classes by bytes usage.
--
-- mysql> select * from x$io_global_by_wait_by_bytes;
-- +-------------------------+-------+---------------+-------------+-------------+--------------+------------+------------+------------+-------------+---------------+-------------+-----------------+
-- | event_name              | total | total_latency | min_latency | avg_latency | max_latency  | count_read | total_read | avg_read   | count_write | total_written | avg_written | total_requested |
-- +-------------------------+-------+---------------+-------------+-------------+--------------+------------+------------+------------+-------------+---------------+-------------+-----------------+
-- | innodb/innodb_data_file |   151 |  334405721910 |     8399560 |  2214607429 | 107444600380 |        147 |    4472832 | 30427.4286 |           0 |             0 |      0.0000 |         4472832 |
-- | sql/FRM                 |   555 |  147752034170 |      674830 |   266219881 |  57705900850 |        270 |     112174 |   415.4593 |           0 |             0 |      0.0000 |          112174 |
-- | innodb/innodb_log_file  |    22 |   56776429970 |     2476890 |  2580746816 |  18883021430 |          6 |      69632 | 11605.3333 |           5 |          2560 |    512.0000 |           72192 |
-- | sql/ERRMSG              |     5 |   11862056180 |    14883960 |  2372411236 |  11109473700 |          3 |      44724 | 14908.0000 |           0 |             0 |      0.0000 |           44724 |
-- | mysys/charset           |     3 |    7256869230 |    19796270 |  2418956410 |   7198498320 |          1 |      18317 | 18317.0000 |           0 |             0 |      0.0000 |           18317 |
-- | myisam/kfile            |   135 |   10194698280 |      784160 |    75516283 |   2593514950 |         40 |       9216 |   230.4000 |          33 |          1017 |     30.8182 |           10233 |
-- | myisam/dfile            |    68 |   10527909730 |      772850 |   154822201 |   7600014630 |          9 |       6667 |   740.7778 |           0 |             0 |      0.0000 |            6667 |
-- | sql/pid                 |     3 |     216507330 |    41296580 |    72169110 |    100617530 |          0 |          0 |     0.0000 |           1 |             6 |      6.0000 |               6 |
-- | sql/casetest            |     5 |     185261570 |     4105530 |    37052314 |    113488310 |          0 |          0 |     0.0000 |           0 |             0 |      0.0000 |               0 |
-- | sql/global_ddl_log      |     2 |      21538010 |     3121560 |    10769005 |     18416450 |          0 |          0 |     0.0000 |           0 |             0 |      0.0000 |               0 |
-- | sql/dbopt               |    10 |    1004267680 |     1164930 |   100426768 |    939894930 |          0 |          0 |     0.0000 |           0 |             0 |      0.0000 |               0 |
-- +-------------------------+-------+---------------+-------------+-------------+--------------+------------+------------+------------+-------------+---------------+-------------+-----------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$io_global_by_wait_by_bytes (
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
SELECT SUBSTRING_INDEX(event_name, '/', -2) AS event_name,
       count_star AS total,
       sum_timer_wait AS total_latency,
       min_timer_wait AS min_latency,
       avg_timer_wait AS avg_latency,
       max_timer_wait AS max_latency,
       count_read,
       sum_number_of_bytes_read AS total_read,
       IFNULL(sum_number_of_bytes_read / NULLIF(count_read, 0), 0) AS avg_read,
       count_write,
       sum_number_of_bytes_write AS total_written,
       IFNULL(sum_number_of_bytes_write / NULLIF(count_write, 0), 0) AS avg_written,
       sum_number_of_bytes_write + sum_number_of_bytes_read AS total_requested
  FROM performance_schema.file_summary_by_event_name
 WHERE event_name LIKE 'wait/io/file/%' 
   AND count_star > 0
 ORDER BY sum_number_of_bytes_write + sum_number_of_bytes_read DESC;
