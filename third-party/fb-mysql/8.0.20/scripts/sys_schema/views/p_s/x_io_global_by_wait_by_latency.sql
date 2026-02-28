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
-- View: x$io_global_by_wait_by_latency
--
-- Shows the top global IO consumers by latency.
--
-- mysql> select * from x$io_global_by_wait_by_latency;
-- +-------------------------+-------+----------------+-------------+--------------+--------------+---------------+----------------+------------+------------+------------+-------------+---------------+-------------+
-- | event_name              | total | total_latency  | avg_latency | max_latency  | read_latency | write_latency | misc_latency   | count_read | total_read | avg_read   | count_write | total_written | avg_written |
-- +-------------------------+-------+----------------+-------------+--------------+--------------+---------------+----------------+------------+------------+------------+-------------+---------------+-------------+
-- | sql/file_parser         |  5945 | 33615441247050 |  5654405471 | 203652881640 |  22093704230 |   27389668280 | 33565957874540 |         26 |       7008 |   269.5385 |         808 |       2479209 |   3068.3280 |
-- | sql/FRM                 |  6332 |  1755386796800 |   277224688 | 145624702340 | 519139578620 |    1677016640 |  1234570201540 |       2040 |     865905 |   424.4632 |         439 |        103445 |    235.6378 |
-- | innodb/innodb_data_file |  1344 |  1522989889460 |  1133176798 | 350700491310 | 203817502460 |  450959403830 |   868212983170 |        147 |    2408448 | 16384.0000 |        1001 |      56213504 |  56157.3467 |
-- | innodb/innodb_log_file  |   828 |   893475794640 |  1079076921 |  30108124800 |  16315236730 |  705886928240 |   171273629670 |          6 |      69632 | 11605.3333 |         413 |       2294272 |   5555.1380 |
-- | myisam/kfile            |  7826 |   246001992860 |    31433883 |  19265276810 |  74419162870 |   23923730090 |   147659099900 |        770 |     141058 |   183.1922 |        4516 |        249602 |     55.2706 |
-- | myisam/dfile            | 13431 |   228191713620 |    16989882 |  32500163410 |  89162969350 |   17341973610 |   121686770660 |       5819 |    4873176 |   837.4594 |        1577 |       2853444 |   1809.4128 |
-- | csv/metadata            |     8 |    28975194560 |  3621899320 |  20148109020 |    399265620 |             0 |    28575928940 |          2 |         70 |    35.0000 |           0 |             0 |      0.0000 |
-- | mysys/charset           |     3 |    24244722970 |  8081574072 |  24151547420 |  24151547420 |             0 |       93175550 |          1 |      17722 | 17722.0000 |           0 |             0 |      0.0000 |
-- | sql/ERRMSG              |     5 |    20427386850 |  4085477370 |  19312386730 |  20324183100 |             0 |      103203750 |          3 |      60390 | 20130.0000 |           0 |             0 |      0.0000 |
-- | mysys/cnf               |     5 |    11366169230 |  2273233846 |  11283602460 |  11287953040 |             0 |       78216190 |          3 |         56 |    18.6667 |           0 |             0 |      0.0000 |
-- | sql/dbopt               |    57 |     4042348570 |    70918224 |    843703380 |            0 |     186430270 |     3855918300 |          0 |          0 |     0.0000 |           7 |           431 |     61.5714 |
-- | csv/data                |     4 |      411548280 |   102887070 |    234886080 |            0 |             0 |      411548280 |          0 |          0 |     0.0000 |           0 |             0 |      0.0000 |
-- | sql/misc                |    24 |      369128240 |    15380092 |     33771660 |            0 |             0 |      369128240 |          0 |          0 |     0.0000 |           0 |             0 |      0.0000 |
-- | archive/data            |    39 |      277856540 |     7124169 |     16180840 |            0 |             0 |      277856540 |          0 |          0 |     0.0000 |           0 |             0 |      0.0000 |
-- | sql/pid                 |     3 |      218026640 |    72675421 |    154841440 |            0 |      21639800 |      196386840 |          0 |          0 |     0.0000 |           1 |             6 |      6.0000 |
-- | sql/casetest            |     5 |      197152150 |    39430430 |    126310080 |            0 |             0 |      197152150 |          0 |          0 |     0.0000 |           0 |             0 |      0.0000 |
-- | sql/global_ddl_log      |     2 |       14604980 |     7302490 |     12120550 |            0 |             0 |       14604980 |          0 |          0 |     0.0000 |           0 |             0 |      0.0000 |
-- +-------------------------+-------+----------------+-------------+--------------+--------------+---------------+----------------+------------+------------+------------+-------------+---------------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$io_global_by_wait_by_latency (
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
       sum_timer_wait AS total_latency,
       avg_timer_wait AS avg_latency,
       max_timer_wait AS max_latency,
       sum_timer_read AS read_latency,
       sum_timer_write AS write_latency,
       sum_timer_misc AS misc_latency,
       count_read,
       sum_number_of_bytes_read AS total_read,
       IFNULL(sum_number_of_bytes_read / NULLIF(count_read, 0), 0) AS avg_read,
       count_write,
       sum_number_of_bytes_write AS total_written,
       IFNULL(sum_number_of_bytes_write / NULLIF(count_write, 0), 0) AS avg_written
  FROM performance_schema.file_summary_by_event_name 
 WHERE event_name LIKE 'wait/io/file/%'
   AND count_star > 0
 ORDER BY sum_timer_wait DESC;
