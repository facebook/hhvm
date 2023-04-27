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
-- View: x$statements_with_errors_or_warnings
--
-- Lists all normalized statements that have raised errors or warnings.
--
-- mysql> select * from x$statements_with_errors_or_warnings LIMIT 1\G
-- *************************** 1. row ***************************
--       query: CREATE OR REPLACE ALGORITHM = TEMPTABLE DEFINER = ? @ ? SQL SECURITY INVOKER VIEW ... truncated
--          db: sys
--  exec_count: 2
--      errors: 1
--   error_pct: 50.0000
--    warnings: 0
-- warning_pct: 0.0000
--  first_seen: 2014-03-07 12:56:54
--   last_seen: 2014-03-07 13:01:01
--      digest: 943a788859e623d5f7798ba0ae0fd8a9
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$statements_with_errors_or_warnings (
  query,
  db,
  exec_count,
  errors,
  error_pct,
  warnings,
  warning_pct,
  first_seen,
  last_seen,
  digest
) AS
SELECT DIGEST_TEXT AS query,
       SCHEMA_NAME as db,
       COUNT_STAR AS exec_count,
       SUM_ERRORS AS errors,
       IFNULL(SUM_ERRORS / NULLIF(COUNT_STAR, 0), 0) * 100 as error_pct,
       SUM_WARNINGS AS warnings,
       IFNULL(SUM_WARNINGS / NULLIF(COUNT_STAR, 0), 0) * 100 as warning_pct,
       FIRST_SEEN as first_seen,
       LAST_SEEN as last_seen,
       DIGEST AS digest
  FROM performance_schema.events_statements_summary_by_digest
 WHERE SUM_ERRORS > 0
    OR SUM_WARNINGS > 0
ORDER BY SUM_ERRORS DESC, SUM_WARNINGS DESC;
