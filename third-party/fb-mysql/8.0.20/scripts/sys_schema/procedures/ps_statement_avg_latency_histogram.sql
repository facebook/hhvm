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

DROP PROCEDURE IF EXISTS ps_statement_avg_latency_histogram;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE ps_statement_avg_latency_histogram ()
    COMMENT '
Description
-----------

Outputs a textual histogram graph of the average latency values
across all normalized queries tracked within the Performance Schema
events_statements_summary_by_digest table.

Can be used to show a very high level picture of what kind of 
latency distribution statements running within this instance have.

Parameters
-----------

None.

Example
-----------

mysql> CALL sys.ps_statement_avg_latency_histogram()\\G
*************************** 1. row ***************************
Performance Schema Statement Digest Average Latency Histogram:

  . = 1 unit
  * = 2 units
  # = 3 units

(0 - 38ms)     240 | ################################################################################
(38 - 77ms)    38  | ......................................
(77 - 115ms)   3   | ...
(115 - 154ms)  62  | *******************************
(154 - 192ms)  3   | ...
(192 - 231ms)  0   |
(231 - 269ms)  0   |
(269 - 307ms)  0   |
(307 - 346ms)  0   |
(346 - 384ms)  1   | .
(384 - 423ms)  1   | .
(423 - 461ms)  0   |
(461 - 499ms)  0   |
(499 - 538ms)  0   |
(538 - 576ms)  0   |
(576 - 615ms)  1   | .

  Total Statements: 350; Buckets: 16; Bucket Size: 38 ms;
'
    SQL SECURITY INVOKER
    NOT DETERMINISTIC
    READS SQL DATA
BEGIN
SELECT CONCAT('\n',
       '\n  . = 1 unit',
       '\n  * = 2 units',
       '\n  # = 3 units\n',
       @label := CONCAT(@label_inner := CONCAT('\n(0 - ',
                                               ROUND((@bucket_size := (SELECT ROUND((MAX(avg_us) - MIN(avg_us)) / (@buckets := 16)) AS size
                                                                         FROM sys.x$ps_digest_avg_latency_distribution)) / (@unit_div := 1000)),
                                                (@unit := 'ms'), ')'),
                        REPEAT(' ', (@max_label_size := ((1 + LENGTH(ROUND((@bucket_size * 15) / @unit_div)) + 3 + LENGTH(ROUND(@bucket_size * 16) / @unit_div)) + 1)) - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us <= @bucket_size), 0)),
       REPEAT(' ', (@max_label_len := (@max_label_size + LENGTH((@total_queries := (SELECT SUM(cnt) FROM sys.x$ps_digest_avg_latency_distribution)))) + 1) - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < (@one_unit := 40), '.', IF(@count_in_bucket < (@two_unit := 80), '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),

       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND(@bucket_size / @unit_div), ' - ', ROUND((@bucket_size * 2) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size AND b1.avg_us <= @bucket_size * 2), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 2) / @unit_div), ' - ', ROUND((@bucket_size * 3) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 2 AND b1.avg_us <= @bucket_size * 3), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 3) / @unit_div), ' - ', ROUND((@bucket_size * 4) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 3 AND b1.avg_us <= @bucket_size * 4), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 4) / @unit_div), ' - ', ROUND((@bucket_size * 5) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 4 AND b1.avg_us <= @bucket_size * 5), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 5) / @unit_div), ' - ', ROUND((@bucket_size * 6) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 5 AND b1.avg_us <= @bucket_size * 6), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 6) / @unit_div), ' - ', ROUND((@bucket_size * 7) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 6 AND b1.avg_us <= @bucket_size * 7), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 7) / @unit_div), ' - ', ROUND((@bucket_size * 8) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 7 AND b1.avg_us <= @bucket_size * 8), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 8) / @unit_div), ' - ', ROUND((@bucket_size * 9) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 8 AND b1.avg_us <= @bucket_size * 9), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 9) / @unit_div), ' - ', ROUND((@bucket_size * 10) / @unit_div), @unit, ')'),
                         REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                         @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                       FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                      WHERE b1.avg_us > @bucket_size * 9 AND b1.avg_us <= @bucket_size * 10), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 10) / @unit_div), ' - ', ROUND((@bucket_size * 11) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 10 AND b1.avg_us <= @bucket_size * 11), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 11) / @unit_div), ' - ', ROUND((@bucket_size * 12) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 11 AND b1.avg_us <= @bucket_size * 12), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 12) / @unit_div), ' - ', ROUND((@bucket_size * 13) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 12 AND b1.avg_us <= @bucket_size * 13), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 13) / @unit_div), ' - ', ROUND((@bucket_size * 14) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 13 AND b1.avg_us <= @bucket_size * 14), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 14) / @unit_div), ' - ', ROUND((@bucket_size * 15) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 14 AND b1.avg_us <= @bucket_size * 15), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),
       @label := CONCAT(@label_inner := CONCAT('\n(', ROUND((@bucket_size * 15) / @unit_div), ' - ', ROUND((@bucket_size * 16) / @unit_div), @unit, ')'),
                        REPEAT(' ', @max_label_size - LENGTH(@label_inner)),
                        @count_in_bucket := IFNULL((SELECT SUM(cnt)
                                                      FROM sys.x$ps_digest_avg_latency_distribution AS b1 
                                                     WHERE b1.avg_us > @bucket_size * 15 AND b1.avg_us <= @bucket_size * 16), 0)),
       REPEAT(' ', @max_label_len - LENGTH(@label)), '| ',
       IFNULL(REPEAT(IF(@count_in_bucket < @one_unit, '.', IF(@count_in_bucket < @two_unit, '*', '#')), 
       	             IF(@count_in_bucket < @one_unit, @count_in_bucket,
       	             	IF(@count_in_bucket < @two_unit, @count_in_bucket / 2, @count_in_bucket / 3))), ''),

       '\n\n  Total Statements: ', @total_queries, '; Buckets: ', @buckets , '; Bucket Size: ', ROUND(@bucket_size / @unit_div) , ' ', @unit, ';\n'

      ) AS `Performance Schema Statement Digest Average Latency Histogram`;

END $$

DELIMITER ;
