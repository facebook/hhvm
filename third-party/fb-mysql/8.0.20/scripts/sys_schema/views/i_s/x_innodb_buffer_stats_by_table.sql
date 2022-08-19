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
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

--
-- View: x$innodb_buffer_stats_by_table
-- 
-- Summarizes the output of the INFORMATION_SCHEMA.INNODB_BUFFER_PAGE 
-- table, aggregating by schema and table name
--
-- mysql> select * from x$innodb_buffer_stats_by_table;
-- +--------------------------+------------------------------------+-----------+--------+-------+--------------+-----------+-------------+
-- | object_schema            | object_name                        | allocated | data   | pages | pages_hashed | pages_old | rows_cached |
-- +--------------------------+------------------------------------+-----------+--------+-------+--------------+-----------+-------------+
-- | InnoDB System            | SYS_COLUMNS                        |    131072 | 101350 |     8 |            8 |         8 |        1532 |
-- | InnoDB System            | SYS_FOREIGN                        |    131072 |  56808 |     8 |            8 |         8 |         172 |
-- | InnoDB System            | SYS_TABLES                         |    131072 |  57529 |     8 |            8 |         8 |         365 |
-- | InnoDB System            | SYS_INDEXES                        |    114688 |  77984 |     7 |            7 |         7 |        1046 |
-- | mem30_trunk__instruments | agentlatencytime                   |     98304 |  29517 |     6 |            6 |         6 |         252 |
-- | mem30_trunk__instruments | binlogspaceusagedata               |     98304 |  23076 |     6 |            6 |         6 |         196 |
-- | mem30_trunk__instruments | connectionsdata                    |     98304 |  37563 |     6 |            6 |         6 |         276 |
-- ...
-- +--------------------------+------------------------------------+-----------+--------+-------+--------------+-----------+-------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$innodb_buffer_stats_by_table (
  object_schema,
  object_name,
  allocated,
  data,
  pages,
  pages_hashed,
  pages_old,
  rows_cached
) AS
SELECT IF(LOCATE('.', ibp.table_name) = 0, 'InnoDB System', REPLACE(SUBSTRING_INDEX(ibp.table_name, '.', 1), '`', '')) AS object_schema,
       REPLACE(SUBSTRING_INDEX(ibp.table_name, '.', -1), '`', '') AS object_name,
       SUM(IF(ibp.compressed_size = 0, 16384, compressed_size)) AS allocated,
       SUM(ibp.data_size) AS data,
       COUNT(ibp.page_number) AS pages,
       COUNT(IF(ibp.is_hashed = 'YES', 1, NULL)) AS pages_hashed,
       COUNT(IF(ibp.is_old = 'YES', 1, NULL)) AS pages_old,
       ROUND(IFNULL(SUM(ibp.number_records)/NULLIF(COUNT(DISTINCT ibp.index_name), 0), 0)) AS rows_cached 
  FROM information_schema.innodb_buffer_page ibp 
 WHERE table_name IS NOT NULL
 GROUP BY object_schema, object_name
 ORDER BY SUM(IF(ibp.compressed_size = 0, 16384, compressed_size)) DESC;
