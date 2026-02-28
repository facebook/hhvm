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
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

--
-- View: x$memory_by_host_by_current_bytes
--
-- Summarizes memory use by host
-- 
-- When the host found is NULL, it is assumed to be a local "background" thread.  
--
-- mysql> select * from x$memory_by_host_by_current_bytes;
-- +------------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- | host       | current_count_used | current_allocated | current_avg_alloc | current_max_alloc | total_allocated |
-- +------------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- | background |               2773 |          11362444 |         4097.5276 |           8390792 |        32184183 |
-- | localhost  |               1508 |            813040 |          539.1512 |            180616 |        88168182 |
-- +------------+--------------------+-------------------+-------------------+-------------------+-----------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$memory_by_host_by_current_bytes (
  host,
  current_count_used,
  current_allocated,
  current_avg_alloc,
  current_max_alloc,
  total_allocated
) AS
SELECT IF(host IS NULL, 'background', host) AS host,
       SUM(current_count_used) AS current_count_used,
       SUM(current_number_of_bytes_used) AS current_allocated,
       IFNULL(SUM(current_number_of_bytes_used) / NULLIF(SUM(current_count_used), 0), 0) AS current_avg_alloc,
       MAX(current_number_of_bytes_used) AS current_max_alloc,
       SUM(sum_number_of_bytes_alloc) AS total_allocated
  FROM performance_schema.memory_summary_by_host_by_event_name
 GROUP BY IF(host IS NULL, 'background', host)
 ORDER BY SUM(current_number_of_bytes_used) DESC;
