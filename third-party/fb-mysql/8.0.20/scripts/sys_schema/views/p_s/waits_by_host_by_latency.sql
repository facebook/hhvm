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
-- View: waits_by_host_by_latency
--
-- Lists the top wait events per host by their total latency, ignoring idle (this may be very large).
--
-- mysql> select * from sys.waits_by_host_by_latency where host != 'background' limit 5;
-- +-----------+------------------------------+-------+---------------+-------------+-------------+
-- | host      | event                        | total | total_latency | avg_latency | max_latency |
-- +-----------+------------------------------+-------+---------------+-------------+-------------+
-- | localhost | wait/io/file/sql/file_parser |  1386 | 14.50 s       | 10.46 ms    | 357.36 ms   |
-- | localhost | wait/io/file/sql/FRM         |   162 | 356.08 ms     | 2.20 ms     | 75.33 ms    |
-- | localhost | wait/io/file/myisam/kfile    |   410 | 322.29 ms     | 786.08 us   | 65.98 ms    |
-- | localhost | wait/io/file/myisam/dfile    |  1327 | 307.44 ms     | 231.68 us   | 37.16 ms    |
-- | localhost | wait/io/file/sql/dbopt       |    89 | 180.34 ms     | 2.03 ms     | 63.41 ms    |
-- +-----------+------------------------------+-------+---------------+-------------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW waits_by_host_by_latency (
  host,
  event,
  total,
  total_latency,
  avg_latency,
  max_latency
) AS
SELECT IF(host IS NULL, 'background', host) AS host,
       event_name AS event,
       count_star AS total,
       format_pico_time(sum_timer_wait) AS total_latency,
       format_pico_time(avg_timer_wait) AS avg_latency,
       format_pico_time(max_timer_wait) AS max_latency
  FROM performance_schema.events_waits_summary_by_host_by_event_name
 WHERE event_name != 'idle'
   AND sum_timer_wait > 0
 ORDER BY host, sum_timer_wait DESC;
