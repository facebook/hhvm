--
-- fb_ps_schema_table_statistics_io is a view used internally in Facebook. It only contains rocksdb and innodb io statistics
-- fb_ps_schema_table_statistics_io view has these columns:
--   table_schema. The schema that contains the table
--   table_name. The table name.
--   io_read. The total number of bytes read from the table.
--   io_read_requests. The total wait time of reads from the table.
--   io_write. The total number of bytes written to the table.
--   io_write_requests. The total number of write requests for the table.
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER
VIEW fb_ps_schema_table_statistics_io (
  table_schema,
  table_name,
  io_read_requests,
  io_read,
  io_write_requests,
  io_write
) AS

SELECT psts.object_schema AS table_schema,
       psts.object_name AS table_name,
       IFNULL(fsbi.count_read, psts.io_read_requests) AS io_read_requests,
       IFNULL(fsbi.sum_number_of_bytes_read, psts.io_read_bytes) AS io_read,
       IFNULL(fsbi.count_write, psts.io_write_requests) AS io_write_requests,
       IFNULL(fsbi.sum_number_of_bytes_write, psts.io_write_bytes) AS io_write
  FROM performance_schema.table_statistics_by_table AS psts
  LEFT JOIN (
       SELECT extract_schema_from_file_name_linux(file_name) COLLATE utf8mb4_0900_ai_ci AS table_schema,
              extract_table_from_file_name_linux(file_name) COLLATE utf8mb4_0900_ai_ci AS table_name,
              count_read,
              sum_number_of_bytes_read,
              count_write,
              sum_number_of_bytes_write
         FROM performance_schema.file_summary_by_instance
        WHERE event_name="wait/io/file/innodb/innodb_data_file"
        union (select 1, 2, 3, 4, 5, 6 limit 0) )
    AS fsbi
    ON psts.object_schema = fsbi.table_schema
   AND psts.object_name = fsbi.table_name;
