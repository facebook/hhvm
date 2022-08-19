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
-- mysql> select * from sys.x$waits_by_host_by_latency where host != 'background' limit 5;
-- +-----------+------------------------------+-------+----------------+-------------+--------------+
-- | host      | event                        | total | total_latency  | avg_latency | max_latency  |
-- +-----------+------------------------------+-------+----------------+-------------+--------------+
-- | localhost | wait/io/file/sql/file_parser |  1388 | 14502657551590 | 10448600240 | 357364034170 |
-- | localhost | wait/io/file/sql/FRM         |   167 |   361060236420 |  2162037319 |  75331088170 |
-- | localhost | wait/io/file/myisam/kfile    |   410 |   322294755250 |   786084585 |  65978227120 |
-- | localhost | wait/io/file/myisam/dfile    |  1327 |   307435262550 |   231676679 |  37162925800 |
-- | localhost | wait/io/file/sql/dbopt       |    89 |   180341976360 |  2026314303 |  63405386850 |
-- +-----------+------------------------------+-------+----------------+-------------+--------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$waits_by_host_by_latency (
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
       sum_timer_wait AS total_latency,
       avg_timer_wait AS avg_latency,
       max_timer_wait AS max_latency
  FROM performance_schema.events_waits_summary_by_host_by_event_name
 WHERE event_name != 'idle'
   AND sum_timer_wait > 0
 ORDER BY host, sum_timer_wait DESC;
