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
-- View: x$memory_by_user_by_current_bytes
--
-- Summarizes memory use by user
-- 
-- When the user found is NULL, it is assumed to be a "background" thread.  
--
-- mysql> select * from x$memory_by_user_by_current_bytes;
-- +------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- | user | current_count_used | current_allocated | current_avg_alloc | current_max_alloc | total_allocated |
-- +------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- | root |               1399 |           1124553 |          803.8263 |            343008 |        45426133 |
-- | mark |                201 |            507990 |         2527.3134 |            343008 |         5769804 |
-- +------+--------------------+-------------------+-------------------+-------------------+-----------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$memory_by_user_by_current_bytes (
  user,
  current_count_used,
  current_allocated,
  current_avg_alloc,
  current_max_alloc,
  total_allocated
) AS
SELECT IF(user IS NULL, 'background', user) AS user,
       SUM(current_count_used) AS current_count_used,
       SUM(current_number_of_bytes_used) AS current_allocated,
       IFNULL(SUM(current_number_of_bytes_used) / NULLIF(SUM(current_count_used), 0), 0) AS current_avg_alloc,
       MAX(current_number_of_bytes_used) AS current_max_alloc,
       SUM(sum_number_of_bytes_alloc) AS total_allocated
  FROM performance_schema.memory_summary_by_user_by_event_name
 GROUP BY IF(user IS NULL, 'background', user)
 ORDER BY SUM(current_number_of_bytes_used) DESC;
