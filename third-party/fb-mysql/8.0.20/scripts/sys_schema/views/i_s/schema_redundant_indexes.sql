-- Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
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
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

--
-- View: schema_redundant_keys
--
-- Shows indexes which are made redundant (or duplicate) by other (dominant) keys.
--
-- mysql> select * from sys.schema_redundant_indexes\G
-- *************************** 1. row ***************************
--               table_schema: test
--                 table_name: rkey
--       redundant_index_name: j
--    redundant_index_columns: j
-- redundant_index_non_unique: 1
--        dominant_index_name: j_2
--     dominant_index_columns: j,k
--  dominant_index_non_unique: 1
--             subpart_exists: 0
--             sql_drop_index: ALTER TABLE `test`.`rkey` DROP INDEX `j`
-- 1 row in set (0.20 sec)
-- 
-- mysql> SHOW CREATE TABLE test.rkey\G
-- *************************** 1. row ***************************
--        Table: rkey
-- Create Table: CREATE TABLE `rkey` (
--   `i` int NOT NULL,
--   `j` int DEFAULT NULL,
--   `k` int DEFAULT NULL,
--   PRIMARY KEY (`i`),
--   KEY `j` (`j`),
--   KEY `j_2` (`j`,`k`)
-- ) ENGINE=InnoDB DEFAULT CHARSET=latin1
-- 1 row in set (0.06 sec)
-- 

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER
VIEW schema_redundant_indexes (
  table_schema,
  table_name,
  redundant_index_name,
  redundant_index_columns,
  redundant_index_non_unique,
  dominant_index_name,
  dominant_index_columns,
  dominant_index_non_unique,
  subpart_exists,
  sql_drop_index
) AS
  SELECT
    redundant_keys.table_schema,
    redundant_keys.table_name,
    redundant_keys.index_name AS redundant_index_name,
    redundant_keys.index_columns AS redundant_index_columns,
    redundant_keys.non_unique AS redundant_index_non_unique,
    dominant_keys.index_name AS dominant_index_name,
    dominant_keys.index_columns AS dominant_index_columns,
    dominant_keys.non_unique AS dominant_index_non_unique,
    IF(redundant_keys.subpart_exists OR dominant_keys.subpart_exists, 1 ,0) AS subpart_exists,
    CONCAT(
      'ALTER TABLE `', redundant_keys.table_schema, '`.`', redundant_keys.table_name, '` DROP INDEX `', redundant_keys.index_name, '`'
      ) AS sql_drop_index
  FROM
    x$schema_flattened_keys AS redundant_keys
    INNER JOIN x$schema_flattened_keys AS dominant_keys
    USING (TABLE_SCHEMA, TABLE_NAME)
  WHERE
    redundant_keys.index_name != dominant_keys.index_name
    AND (
      ( 
        /* Identical columns */
        (redundant_keys.index_columns = dominant_keys.index_columns)
        AND (
          (redundant_keys.non_unique > dominant_keys.non_unique)
          OR (redundant_keys.non_unique = dominant_keys.non_unique 
          	AND IF(redundant_keys.index_name='PRIMARY', '', redundant_keys.index_name) > IF(dominant_keys.index_name='PRIMARY', '', dominant_keys.index_name)
          )
        )
      )
      OR
      ( 
        /* Non-unique prefix columns */
        LOCATE(CONCAT(redundant_keys.index_columns, ','), dominant_keys.index_columns) = 1
        AND redundant_keys.non_unique = 1
      )
      OR
      ( 
        /* Unique prefix columns */
        LOCATE(CONCAT(dominant_keys.index_columns, ','), redundant_keys.index_columns) = 1
        AND dominant_keys.non_unique = 0
      )
    );
