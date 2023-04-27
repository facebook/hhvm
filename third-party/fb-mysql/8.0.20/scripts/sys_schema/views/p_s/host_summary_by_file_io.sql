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
-- View: host_summary_by_file_io
--
-- Summarizes file IO totals per host.
--
-- When the host found is NULL, it is assumed to be a "background" thread.
--
-- mysql> select * from host_summary_by_file_io;
-- +------------+-------+------------+
-- | host       | ios   | io_latency |
-- +------------+-------+------------+
-- | hal1       | 26457 | 21.58 s    |
-- | hal2       |  1189 | 394.21 ms  |
-- +------------+-------+------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW host_summary_by_file_io (
  host,
  ios,
  io_latency
) AS
SELECT IF(host IS NULL, 'background', host) AS host,
       SUM(count_star) AS ios,
       format_pico_time(SUM(sum_timer_wait)) AS io_latency 
  FROM performance_schema.events_waits_summary_by_host_by_event_name
 WHERE event_name LIKE 'wait/io/file/%'
 GROUP BY IF(host IS NULL, 'background', host)
 ORDER BY SUM(sum_timer_wait) DESC;
