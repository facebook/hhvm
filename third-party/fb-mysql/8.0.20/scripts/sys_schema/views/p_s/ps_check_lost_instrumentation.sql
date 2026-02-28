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
-- View: ps_check_lost_instrumentation
-- 
-- Used to check whether Performance Schema is not able to monitor
-- all runtime data - only returns variables that have lost instruments
--
-- mysql> select * from ps_check_lost_instrumentation;
-- +----------------------------------------+----------------+
-- | variable_name                          | variable_value |
-- +----------------------------------------+----------------+
-- | Performance_schema_file_handles_lost   | 101223         |
-- | Performance_schema_file_instances_lost | 1231           |
-- +----------------------------------------+----------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW ps_check_lost_instrumentation (
  variable_name,
  variable_value
)
AS
SELECT variable_name, variable_value
  FROM performance_schema.global_status
 WHERE variable_name LIKE 'perf%lost'
   AND variable_value > 0;
