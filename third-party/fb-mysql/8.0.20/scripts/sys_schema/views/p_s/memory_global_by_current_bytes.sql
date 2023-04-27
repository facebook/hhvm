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
-- View: memory_global_by_current_bytes
-- 
-- Shows the current memory usage within the server globally broken down by allocation type.
--
-- mysql> select * from memory_global_by_current_bytes;
-- +-------------------------------------------------+---------------+---------------+-------------------+------------+-------------+----------------+
-- | event_name                                      | current_count | current_alloc | current_avg_alloc | high_count | high_alloc  | high_avg_alloc |
-- +-------------------------------------------------+---------------+---------------+-------------------+------------+-------------+----------------+
-- | memory/performance_schema/internal_buffers      |            62 | 293.80 MiB    | 4.74 MiB          |         62 | 293.80 MiB  | 4.74 MiB       |
-- | memory/innodb/buf_buf_pool                      |             1 | 131.06 MiB    | 131.06 MiB        |          1 | 131.06 MiB  | 131.06 MiB     |
-- | memory/innodb/log0log                           |             9 | 8.01 MiB      | 911.15 KiB        |          9 | 8.01 MiB    | 911.15 KiB     |
-- | memory/mysys/KEY_CACHE                          |             3 | 8.00 MiB      | 2.67 MiB          |          3 | 8.00 MiB    | 2.67 MiB       |
-- | memory/innodb/hash0hash                         |            27 | 4.73 MiB      | 179.51 KiB        |         27 | 6.84 MiB    | 259.47 KiB     |
-- | memory/innodb/os0event                          |         24998 | 4.01 MiB      | 168 bytes         |      24998 | 4.01 MiB    | 168 bytes      |
-- ...
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW memory_global_by_current_bytes (
  event_name,
  current_count,
  current_alloc,
  current_avg_alloc,
  high_count,
  high_alloc,
  high_avg_alloc
) AS
SELECT event_name,
       current_count_used AS current_count,
       format_bytes(current_number_of_bytes_used) AS current_alloc,
       format_bytes(IFNULL(current_number_of_bytes_used / NULLIF(current_count_used, 0), 0)) AS current_avg_alloc,
       high_count_used AS high_count,
       format_bytes(high_number_of_bytes_used) AS high_alloc,
       format_bytes(IFNULL(high_number_of_bytes_used / NULLIF(high_count_used, 0), 0)) AS high_avg_alloc
  FROM performance_schema.memory_summary_global_by_event_name
 WHERE current_number_of_bytes_used > 0
 ORDER BY current_number_of_bytes_used DESC;
