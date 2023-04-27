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
-- View: x$schema_flattened_keys
--
-- Helper view for the schema_redundant_keys view.
--
-- mysql> select * from sys.x$schema_flattened_keys;
-- +---------------+---------------------+------------------------------+------------+----------------+-----------------+
-- | table_schema  | table_name          | index_name                   | non_unique | subpart_exists | index_columns   |
-- +---------------+---------------------+------------------------------+------------+----------------+-----------------+
-- | mem__advisors | advisor_initialized | PRIMARY                      |          0 |              0 | advisorClassId  |
-- | mem__advisors | advisor_schedules   | advisorClassIdIdx            |          1 |              0 | advisorClassId  |
-- | mem__advisors | advisor_schedules   | PRIMARY                      |          0 |              0 | schedule_id     |
-- | mem__advisors | app_identity_path   | FK_7xbq2i81hgo0xlvnb6rr77s21 |          1 |              0 | for_schedule_id |
-- | mem__advisors | app_identity_path   | PRIMARY                      |          0 |              0 | hib_id          |
-- ...
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER
VIEW x$schema_flattened_keys (
  table_schema,
  table_name,
  index_name,
  non_unique,
  subpart_exists,
  index_columns
) AS
  SELECT
    TABLE_SCHEMA,
    TABLE_NAME,
    INDEX_NAME,
    MAX(NON_UNIQUE) AS non_unique,
    MAX(IF(SUB_PART IS NULL, 0, 1)) AS subpart_exists,
    GROUP_CONCAT(COLUMN_NAME ORDER BY SEQ_IN_INDEX) AS index_columns
  FROM INFORMATION_SCHEMA.STATISTICS
  WHERE
    INDEX_TYPE='BTREE'
    AND TABLE_SCHEMA NOT IN ('mysql', 'sys', 'INFORMATION_SCHEMA', 'PERFORMANCE_SCHEMA')
  GROUP BY
    TABLE_SCHEMA, TABLE_NAME, INDEX_NAME;
