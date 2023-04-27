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
-- View: x$ps_digest_95th_percentile_by_avg_us
--
-- Helper view for statements_with_runtimes_in_95th_percentile.
-- Lists the 95th percentile runtime, for all statements
--
-- mysql> select * from x$ps_digest_95th_percentile_by_avg_us;
-- +--------+------------+
-- | avg_us | percentile |
-- +--------+------------+
-- |    964 |     0.9525 |
-- +--------+------------+
--

CREATE OR REPLACE
  ALGORITHM = TEMPTABLE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$ps_digest_95th_percentile_by_avg_us (
  avg_us,
  percentile
) AS
SELECT s2.avg_us avg_us,
       IFNULL(SUM(s1.cnt)/NULLIF((SELECT COUNT(*) FROM performance_schema.events_statements_summary_by_digest), 0), 0) percentile
  FROM sys.x$ps_digest_avg_latency_distribution AS s1
  JOIN sys.x$ps_digest_avg_latency_distribution AS s2
    ON s1.avg_us <= s2.avg_us
 GROUP BY s2.avg_us
HAVING IFNULL(SUM(s1.cnt)/NULLIF((SELECT COUNT(*) FROM performance_schema.events_statements_summary_by_digest), 0), 0) > 0.95
 ORDER BY percentile
 LIMIT 1;
