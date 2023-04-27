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
-- View: x$schema_table_statistics_with_buffer
--
-- Statistics around tables.
--
-- Ordered by the total wait time descending - top tables are most contended.
--
-- More statistics such as caching stats for the InnoDB buffer pool with InnoDB tables
--
-- mysql> SELECT * FROM x$schema_table_statistics_with_buffer LIMIT 1\G
-- *************************** 1. row ***************************
--               table_schema: common_schema
--                 table_name: help_content
--               rows_fetched: 0
--              fetch_latency: 0
--              rows_inserted: 169
--             insert_latency: 409815527680
--               rows_updated: 0
--             update_latency: 0
--               rows_deleted: 0
--             delete_latency: 0
--           io_read_requests: 14
--                    io_read: 1180
--            io_read_latency: 52406770
--          io_write_requests: 131
--                   io_write: 11719246
--           io_write_latency: 133726902790
--           io_misc_requests: 61
--            io_misc_latency: 209081089750
--    innodb_buffer_allocated: 688128
--         innodb_buffer_data: 423667
--        innodb_buffer_pages: 42
-- innodb_buffer_pages_hashed: 42
--    innodb_buffer_pages_old: 42
--  innodb_buffer_rows_cached: 210
--
 
CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$schema_table_statistics_with_buffer (
  table_schema,
  table_name,
  rows_fetched,
  fetch_latency,
  rows_inserted,
  insert_latency,
  rows_updated,
  update_latency,
  rows_deleted,
  delete_latency,
  io_read_requests,
  io_read,
  io_read_latency,
  io_write_requests,
  io_write,
  io_write_latency,
  io_misc_requests,
  io_misc_latency,
  innodb_buffer_allocated,
  innodb_buffer_data,
  innodb_buffer_free,
  innodb_buffer_pages,
  innodb_buffer_pages_hashed,
  innodb_buffer_pages_old,
  innodb_buffer_rows_cached
) AS
SELECT pst.object_schema AS table_schema,
       pst.object_name AS table_name,
       pst.count_fetch AS rows_fetched,
       pst.sum_timer_fetch AS fetch_latency,
       pst.count_insert AS rows_inserted,
       pst.sum_timer_insert AS insert_latency,
       pst.count_update AS rows_updated,
       pst.sum_timer_update AS update_latency,
       pst.count_delete AS rows_deleted,
       pst.sum_timer_delete AS delete_latency,
       fsbi.count_read AS io_read_requests,
       fsbi.sum_number_of_bytes_read AS io_read,
       fsbi.sum_timer_read AS io_read_latency,
       fsbi.count_write AS io_write_requests,
       fsbi.sum_number_of_bytes_write AS io_write,
       fsbi.sum_timer_write AS io_write_latency,
       fsbi.count_misc AS io_misc_requests,
       fsbi.sum_timer_misc AS io_misc_latency,
       ibp.allocated AS innodb_buffer_allocated,
       ibp.data AS innodb_buffer_data,
       (ibp.allocated - ibp.data) AS innodb_buffer_free,
       ibp.pages AS innodb_buffer_pages,
       ibp.pages_hashed AS innodb_buffer_pages_hashed,
       ibp.pages_old AS innodb_buffer_pages_old,
       ibp.rows_cached AS innodb_buffer_rows_cached
  FROM performance_schema.table_io_waits_summary_by_table AS pst
  LEFT JOIN x$ps_schema_table_statistics_io AS fsbi
    ON pst.object_schema = fsbi.table_schema
   AND pst.object_name = fsbi.table_name
  LEFT JOIN sys.x$innodb_buffer_stats_by_table AS ibp
    ON pst.object_schema = ibp.object_schema
   AND pst.object_name = ibp.object_name
 ORDER BY pst.sum_timer_wait DESC;
