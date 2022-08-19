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
-- View: memory_by_thread_by_current_bytes
--
-- Summarizes memory use by user using the 5.7 Performance Schema instrumentation.
-- 
-- User shows either the background or foreground user name appropriately.
--
-- mysql> select * from sys.memory_by_thread_by_current_bytes limit 5;
-- +-----------+----------------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- | thread_id | user           | current_count_used | current_allocated | current_avg_alloc | current_max_alloc | total_allocated |
-- +-----------+----------------+--------------------+-------------------+-------------------+-------------------+-----------------+
-- |         1 | sql/main       |              29333 | 166.02 MiB        | 5.80 KiB          | 131.13 MiB        | 196.00 MiB      |
-- |        55 | root@localhost |                175 | 1.04 MiB          | 6.09 KiB          | 350.86 KiB        | 67.37 MiB       |
-- |        58 | root@localhost |                236 | 368.13 KiB        | 1.56 KiB          | 312.05 KiB        | 130.34 MiB      |
-- |       904 | root@localhost |                 32 | 18.00 KiB         | 576 bytes         | 16.00 KiB         | 6.68 MiB        |
-- |       970 | root@localhost |                 12 | 16.80 KiB         | 1.40 KiB          | 16.00 KiB         | 1.20 MiB        |
-- +-----------+----------------+--------------------+-------------------+-------------------+-------------------+-----------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW memory_by_thread_by_current_bytes (
  thread_id,
  user,
  current_count_used,
  current_allocated,
  current_avg_alloc,
  current_max_alloc,
  total_allocated
) AS
SELECT thread_id,
       IF(t.name = 'thread/sql/one_connection', 
          CONCAT(t.processlist_user, '@', t.processlist_host), 
          REPLACE(t.name, 'thread/', '')) user,
       SUM(mt.current_count_used) AS current_count_used,
       format_bytes(SUM(mt.current_number_of_bytes_used)) AS current_allocated,
       format_bytes(IFNULL(SUM(mt.current_number_of_bytes_used) / NULLIF(SUM(current_count_used), 0), 0)) AS current_avg_alloc,
       format_bytes(MAX(mt.current_number_of_bytes_used)) AS current_max_alloc,
       format_bytes(SUM(mt.sum_number_of_bytes_alloc)) AS total_allocated
  FROM performance_schema.memory_summary_by_thread_by_event_name AS mt
  JOIN performance_schema.threads AS t USING (thread_id)
 GROUP BY thread_id, IF(t.name = 'thread/sql/one_connection', 
          CONCAT(t.processlist_user, '@', t.processlist_host), 
          REPLACE(t.name, 'thread/', ''))
 ORDER BY SUM(current_number_of_bytes_used) DESC;
