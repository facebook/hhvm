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
-- View: x$ps_schema_table_statistics_io
--
-- Helper view for schema_table_statistics
-- Having this view with ALGORITHM = TEMPTABLE means MySQL can use the optimizations for
-- materialized views to improve the overall performance.
--
-- mysql> SELECT * FROM x$ps_schema_table_statistics_io LIMIT 1\G
-- *************************** 1. row ***************************
--              table_schema: charsets
--                table_name: Index
--                count_read: 1
--  sum_number_of_bytes_read: 18710
--            sum_timer_read: 20229409070
--               count_write: 0
-- sum_number_of_bytes_write: 0
--           sum_timer_write: 0
--                count_misc: 2
--            sum_timer_misc: 80768480
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$ps_schema_table_statistics_io (
  table_schema,
  table_name,
  count_read,
  sum_number_of_bytes_read,
  sum_timer_read,
  count_write,
  sum_number_of_bytes_write,
  sum_timer_write,
  count_misc,
  sum_timer_misc
) AS
SELECT extract_schema_from_file_name(file_name) COLLATE utf8mb4_0900_ai_ci AS table_schema,
       extract_table_from_file_name(file_name) COLLATE utf8mb4_0900_ai_ci AS table_name,
       SUM(count_read) AS count_read,
       SUM(sum_number_of_bytes_read) AS sum_number_of_bytes_read,
       SUM(sum_timer_read) AS sum_timer_read,
       SUM(count_write) AS count_write,
       SUM(sum_number_of_bytes_write) AS sum_number_of_bytes_write,
       SUM(sum_timer_write) AS sum_timer_write,
       SUM(count_misc) AS count_misc,
       SUM(sum_timer_misc) AS sum_timer_misc
  FROM performance_schema.file_summary_by_instance
 GROUP BY table_schema, table_name;
