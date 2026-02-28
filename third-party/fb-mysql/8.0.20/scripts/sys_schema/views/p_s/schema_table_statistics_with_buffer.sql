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
-- View: schema_table_statistics_with_buffer
--
-- Statistics around tables.
--
-- Ordered by the total wait time descending - top tables are most contended.
--
-- More statistics such as caching stats for the InnoDB buffer pool with InnoDB tables
--
-- mysql> select * from schema_table_statistics_with_buffer limit 1\G
-- *************************** 1. row ***************************
--                  table_schema: mem
--                    table_name: mysqlserver
--                  rows_fetched: 27087
--                 fetch_latency: 442.72 ms
--                 rows_inserted: 2
--                insert_latency: 185.04 us 
--                  rows_updated: 5096
--                update_latency: 1.39 s
--                  rows_deleted: 0
--                delete_latency: 0 ps
--              io_read_requests: 2565
--                 io_read_bytes: 1121627
--               io_read_latency: 10.07 ms
--             io_write_requests: 1691
--                io_write_bytes: 128383
--              io_write_latency: 14.17 ms
--              io_misc_requests: 2698
--               io_misc_latency: 433.66 ms
--           innodb_buffer_pages: 19
--    innodb_buffer_pages_hashed: 19
--       innodb_buffer_pages_old: 19
-- innodb_buffer_bytes_allocated: 311296
--      innodb_buffer_bytes_data: 1924
--     innodb_buffer_rows_cached: 2
--
 
CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW schema_table_statistics_with_buffer (
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
  queries_used,
  queries_empty,
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
       format_pico_time(pst.sum_timer_fetch) AS fetch_latency,
       pst.count_insert AS rows_inserted,
       format_pico_time(pst.sum_timer_insert) AS insert_latency,
       pst.count_update AS rows_updated,
       format_pico_time(pst.sum_timer_update) AS update_latency,
       pst.count_delete AS rows_deleted,
       format_pico_time(pst.sum_timer_delete) AS delete_latency,
       fsbi.count_read AS io_read_requests,
       format_bytes(fsbi.sum_number_of_bytes_read) AS io_read,
       format_pico_time(fsbi.sum_timer_read) AS io_read_latency,
       fsbi.count_write AS io_write_requests,
       format_bytes(fsbi.sum_number_of_bytes_write) AS io_write,
       format_pico_time(fsbi.sum_timer_write) AS io_write_latency,
       fsbi.count_misc AS io_misc_requests,
       format_pico_time(fsbi.sum_timer_misc) AS io_misc_latency,
       psts.QUERIES_USED AS queries_used,
       psts.EMPTY_QUERIES AS queries_empty,
       format_bytes(ibp.allocated) AS innodb_buffer_allocated,
       format_bytes(ibp.data) AS innodb_buffer_data,
       format_bytes(ibp.allocated - ibp.data) AS innodb_buffer_free,
       ibp.pages AS innodb_buffer_pages,
       ibp.pages_hashed AS innodb_buffer_pages_hashed,
       ibp.pages_old AS innodb_buffer_pages_old,
       ibp.rows_cached AS innodb_buffer_rows_cached
  FROM performance_schema.table_io_waits_summary_by_table AS pst
  LEFT JOIN x$ps_schema_table_statistics_io AS fsbi
    ON pst.object_schema = fsbi.table_schema
   AND pst.object_name = fsbi.table_name
  LEFT JOIN performance_schema.table_statistics_by_table AS psts
    ON pst.object_schema = psts.object_schema
   AND pst.object_name = psts.object_name
   AND pst.object_type = psts.object_type
  LEFT JOIN sys.x$innodb_buffer_stats_by_table AS ibp
    ON pst.object_schema = ibp.object_schema
   AND pst.object_name = ibp.object_name
 ORDER BY pst.sum_timer_wait DESC;
