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
-- View: memory_by_host_by_current_bytes
--
-- Summarizes memory use by host using the 5.7 Performance Schema instrumentation.
-- 
-- When the host found is NULL, it is assumed to be a local "background" thread.  
--
-- mysql> select * from memory_by_host_by_current_bytes;
-- +------------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- | host       | current_count_used | current_allocated | current_avg_alloc | current_max_alloc | total_allocated |
-- +------------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- | background |               2773 | 10.84 MiB         | 4.00 KiB          | 8.00 MiB          | 30.69 MiB       |
-- | localhost  |               1509 | 809.30 KiB        | 549 bytes         | 176.38 KiB        | 83.59 MiB       |
-- +------------+--------------------+-------------------+-------------------+-------------------+-----------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW memory_by_host_by_current_bytes (
  host,
  current_count_used,
  current_allocated,
  current_avg_alloc,
  current_max_alloc,
  total_allocated
) AS
SELECT IF(host IS NULL, 'background', host) AS host,
       SUM(current_count_used) AS current_count_used,
       format_bytes(SUM(current_number_of_bytes_used)) AS current_allocated,
       format_bytes(IFNULL(SUM(current_number_of_bytes_used) / NULLIF(SUM(current_count_used), 0), 0)) AS current_avg_alloc,
       format_bytes(MAX(current_number_of_bytes_used)) AS current_max_alloc,
       format_bytes(SUM(sum_number_of_bytes_alloc)) AS total_allocated
  FROM performance_schema.memory_summary_by_host_by_event_name
 GROUP BY IF(host IS NULL, 'background', host)
 ORDER BY SUM(current_number_of_bytes_used) DESC;
