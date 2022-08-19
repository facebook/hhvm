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
-- View: x$memory_by_thread_by_current_bytes
--
-- Summarizes memory use by user
-- 
-- When the user found is NULL, it is assumed to be a "background" thread.  
--
-- mysql> select * from sys.x$memory_by_thread_by_current_bytes limit 5;
-- +-----------+----------------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- | thread_id | user           | current_count_used | current_allocated | current_avg_alloc | current_max_alloc | total_allocated |
-- +-----------+----------------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- |         1 | sql/main       |              29333 |         174089450 |         5934.9351 |         137494528 |       205523135 |
-- |        55 | root@localhost |                173 |           1074664 |         6211.9306 |            359280 |        72248413 |
-- |        58 | root@localhost |                240 |            377099 |         1571.2458 |            319536 |       169483870 |
-- |      1152 | root@localhost |                 30 |             56949 |         1898.3000 |             16391 |         1010024 |
-- |      1154 | root@localhost |                 34 |             56369 |         1657.9118 |             16391 |         1958771 |
-- +-----------+----------------+--------------------+-------------------+-------------------+-------------------+-----------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$memory_by_thread_by_current_bytes (
  thread_id,
  user,
  current_count_used,
  current_allocated,
  current_avg_alloc,
  current_max_alloc,
  total_allocated
) AS
SELECT t.thread_id,
       IF(t.name = 'thread/sql/one_connection', 
          CONCAT(t.processlist_user, '@', t.processlist_host), 
          REPLACE(t.name, 'thread/', '')) user,
       SUM(mt.current_count_used) AS current_count_used,
       SUM(mt.current_number_of_bytes_used) AS current_allocated,
       IFNULL(SUM(mt.current_number_of_bytes_used) / NULLIF(SUM(current_count_used), 0), 0) AS current_avg_alloc,
       MAX(mt.current_number_of_bytes_used) AS current_max_alloc,
       SUM(mt.sum_number_of_bytes_alloc) AS total_allocated
  FROM performance_schema.memory_summary_by_thread_by_event_name AS mt
  JOIN performance_schema.threads AS t USING (thread_id)
 GROUP BY thread_id, IF(t.name = 'thread/sql/one_connection', 
          CONCAT(t.processlist_user, '@', t.processlist_host), 
          REPLACE(t.name, 'thread/', ''))
 ORDER BY SUM(mt.current_number_of_bytes_used) DESC;
