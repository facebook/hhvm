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
-- View: latest_file_io
--
-- Shows the latest file IO, by file / thread.
--
-- mysql> select * from latest_file_io limit 5;
-- +----------------------+----------------------------------------+------------+-----------+-----------+
-- | thread               | file                                   | latency    | operation | requested |
-- +----------------------+----------------------------------------+------------+-----------+-----------+
-- | msandbox@localhost:1 | @@tmpdir/#sqlcf28_1_4e.MYI             | 9.26 us    | write     | 124 bytes |
-- | msandbox@localhost:1 | @@tmpdir/#sqlcf28_1_4e.MYI             | 4.00 us    | write     | 2 bytes   |
-- | msandbox@localhost:1 | @@tmpdir/#sqlcf28_1_4e.MYI             | 56.34 us   | close     | NULL      |
-- | msandbox@localhost:1 | @@tmpdir/#sqlcf28_1_4e.MYD             | 53.93 us   | close     | NULL      |
-- | msandbox@localhost:1 | @@tmpdir/#sqlcf28_1_4e.MYI             | 104.05 ms  | delete    | NULL      |
-- +----------------------+----------------------------------------+------------+-----------+-----------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW latest_file_io (
  thread,
  file,
  latency,
  operation,
  requested
) AS
SELECT IF(id IS NULL, 
             CONCAT(SUBSTRING_INDEX(name, '/', -1), ':', thread_id), 
             CONCAT(user, '@', host, ':', id)
          ) thread, 
       sys.format_path(object_name) file, 
       format_pico_time(timer_wait) AS latency, 
       operation, 
       format_bytes(number_of_bytes) AS requested
  FROM performance_schema.events_waits_history_long 
  JOIN performance_schema.threads USING (thread_id)
  LEFT JOIN information_schema.processlist ON processlist_id = id
 WHERE object_name IS NOT NULL
   AND event_name LIKE 'wait/io/file/%'
 ORDER BY timer_start;
