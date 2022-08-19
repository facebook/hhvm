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
-- View: x$waits_by_user_by_latency
--
-- Lists the top wait events per user by their total latency, ignoring idle (this may be very large).
--
-- mysql> select * from x$waits_by_user_by_latency;
-- +------+-----------------------------------------------------+--------+----------------+-------------+--------------+
-- | user | event                                               | total  | total_latency  | avg_latency | max_latency  |
-- +------+-----------------------------------------------------+--------+----------------+-------------+--------------+
-- | root | wait/io/file/sql/file_parser                        |  13745 | 60462025415480 |  4398837508 | 231881092170 |
-- | root | wait/io/file/innodb/innodb_data_file                |   4699 |  3023248450820 |   643381037 |  46928334180 |
-- | root | wait/io/file/sql/FRM                                |  11467 |  2600067790580 |   226743257 |  61718277920 |
-- | root | wait/io/file/myisam/dfile                           |  26776 |   746701506200 |    27886690 | 308785046960 |
-- | root | wait/io/file/myisam/kfile                           |   7126 |   462661061590 |    64925432 |  88756408780 |
-- | root | wait/io/file/sql/dbopt                              |    179 |   137577467690 |   768589146 |  15457199810 |
-- | root | wait/io/file/csv/metadata                           |      8 |    86599791590 | 10824973666 |  50322529270 |
-- | root | wait/synch/mutex/mysys/IO_CACHE::append_buffer_lock | 798080 |    66461175430 |       82940 |    161028010 |
-- | root | wait/io/file/sql/binlog                             |     19 |    49110632610 |  2584770058 |   9400449760 |
-- | root | wait/io/file/sql/misc                               |     26 |    22380676630 |   860795052 |  15298475270 |
-- | root | wait/io/file/csv/data                               |      4 |      297460540 |    74365135 |    111931300 |
-- | root | wait/synch/rwlock/sql/MDL_lock::rwlock              |    944 |      287862120 |      304616 |       874640 |
-- | root | wait/io/file/archive/data                           |      4 |       82713800 |    20678450 |     40738620 |
-- | root | wait/synch/mutex/myisam/MYISAM_SHARE::intern_lock   |     60 |       12211030 |      203203 |       512720 |
-- | root | wait/synch/mutex/innodb/trx_mutex                   |     81 |        5926440 |       73138 |       252590 |
-- +------+-----------------------------------------------------+--------+----------------+-------------+--------------+
--

CREATE OR REPLACE
  ALGORITHM = MERGE
  DEFINER = 'mysql.sys'@'localhost'
  SQL SECURITY INVOKER 
VIEW x$waits_by_user_by_latency (
  user,
  event,
  total,
  total_latency,
  avg_latency,
  max_latency
) AS
SELECT IF(user IS NULL, 'background', user) AS user,
       event_name AS event,
       count_star AS total,
       sum_timer_wait AS total_latency,
       avg_timer_wait AS avg_latency,
       max_timer_wait AS max_latency
  FROM performance_schema.events_waits_summary_by_user_by_event_name
 WHERE event_name != 'idle'
   AND user IS NOT NULL
   AND sum_timer_wait > 0
 ORDER BY user, sum_timer_wait DESC;
