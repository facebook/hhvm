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
-- View: x$io_global_by_file_by_bytes
--
-- Shows the top global IO consumers by bytes usage by file.
--
-- mysql> SELECT * FROM x$io_global_by_file_by_bytes LIMIT 5;
-- +------------------------------------------------------+------------+------------+------------+-------------+---------------+------------+---------+-----------+
-- | file                                                 | count_read | total_read | avg_read   | count_write | total_written | avg_write  | total   | write_pct |
-- +------------------------------------------------------+------------+------------+------------+-------------+---------------+------------+---------+-----------+
-- | /Users/mark/sandboxes/msb_5_5_33/data/ibdata1        |        147 |    4472832 | 30427.4286 |           3 |         49152 | 16384.0000 | 4521984 |      1.09 |
-- | /Users/mark/sandboxes/msb_5_5_33/data/mysql/proc.MYD |        347 |      87397 |   251.8646 |         111 |         19536 |   176.0000 |  106933 |     18.27 |
-- | /Users/mark/sandboxes/msb_5_5_33/data/ib_logfile0    |          6 |      69632 | 11605.3333 |           8 |          4096 |   512.0000 |   73728 |      5.56 |
-- | /opt/mysql/5.5.33/share/english/errmsg.sys           |          3 |      44724 | 14908.0000 |           0 |             0 |     0.0000 |   44724 |      0.00 |
-- | /opt/mysql/5.5.33/share/charsets/Index.xml           |          1 |      18317 | 18317.0000 |           0 |             0 |     0.0000 |   18317 |      0.00 |
-- +------------------------------------------------------+------------+------------+------------+-------------+---------------+------------+---------+-----------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$io_global_by_file_by_bytes (
  file,
  count_read,
  total_read,
  avg_read,
  count_write,
  total_written,
  avg_write,
  total,
  write_pct
) AS
SELECT file_name AS file, 
       count_read, 
       sum_number_of_bytes_read AS total_read,
       IFNULL(sum_number_of_bytes_read / NULLIF(count_read, 0), 0) AS avg_read,
       count_write, 
       sum_number_of_bytes_write AS total_written,
       IFNULL(sum_number_of_bytes_write / NULLIF(count_write, 0), 0.00) AS avg_write,
       sum_number_of_bytes_read + sum_number_of_bytes_write AS total, 
       IFNULL(ROUND(100-((sum_number_of_bytes_read/ NULLIF((sum_number_of_bytes_read+sum_number_of_bytes_write), 0))*100), 2), 0.00) AS write_pct 
  FROM performance_schema.file_summary_by_instance
 ORDER BY sum_number_of_bytes_read + sum_number_of_bytes_write DESC;
