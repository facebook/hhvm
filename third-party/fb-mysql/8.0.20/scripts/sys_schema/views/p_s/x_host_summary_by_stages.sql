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
-- View: x$host_summary_by_stages
--
-- Summarizes stages by host, ordered by host and total latency per stage.
--
-- When the host found is NULL, it is assumed to be a "background" thread.
--
-- mysql> select * from x$host_summary_by_stages;
-- +------+--------------------------------+-------+---------------+-------------+
-- | host | event_name                     | total | total_latency | avg_latency |
-- +------+--------------------------------+-------+---------------+-------------+
-- | hal  | stage/sql/Opening tables       |  1114 |   71919037000 |    64559000 |
-- | hal  | stage/sql/Creating sort index  |     5 |    2245762000 |   449152000 |
-- | hal  | stage/sql/init                 |    13 |     428798000 |    32984000 |
-- | hal  | stage/sql/checking permissions |    13 |     363231000 |    27940000 |
-- | hal  | stage/sql/freeing items        |     7 |     137728000 |    19675000 |
-- | hal  | stage/sql/statistics           |     6 |      93955000 |    15659000 |
-- | hal  | stage/sql/preparing            |     6 |      82571000 |    13761000 |
-- | hal  | stage/sql/optimizing           |     6 |      63338000 |    10556000 |
-- | hal  | stage/sql/Sending data         |     6 |      53400000 |     8900000 |
-- | hal  | stage/sql/closing tables       |     7 |      46922000 |     6703000 |
-- | hal  | stage/sql/System lock          |     6 |      40175000 |     6695000 |
-- | hal  | stage/sql/query end            |     7 |      31723000 |     4531000 |
-- | hal  | stage/sql/Sorting result       |     6 |       9855000 |     1642000 |
-- | hal  | stage/sql/end                  |     6 |       9556000 |     1592000 |
-- | hal  | stage/sql/cleaning up          |     7 |       7312000 |     1044000 |
-- | hal  | stage/sql/executing            |     6 |       6487000 |     1081000 |
-- +------+--------------------------------+-------+---------------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$host_summary_by_stages (
  host,
  event_name,
  total,
  total_latency,
  avg_latency
) AS
SELECT IF(host IS NULL, 'background', host) AS host,
       event_name,
       count_star AS total,
       sum_timer_wait AS total_latency, 
       avg_timer_wait AS avg_latency 
  FROM performance_schema.events_stages_summary_by_host_by_event_name
 WHERE sum_timer_wait != 0
 ORDER BY IF(host IS NULL, 'background', host), sum_timer_wait DESC;
