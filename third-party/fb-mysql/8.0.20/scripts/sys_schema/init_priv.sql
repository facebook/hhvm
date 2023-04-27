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

REPLACE INTO mysql.user VALUES
('localhost','mysql.sys','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','','','','',0,0,0,0,'caching_sha2_password','\$A\$005\$THISISACOMBINATIONOFINVALIDSALTANDPASSWORDTHATMUSTNEVERBRBEUSED','N',CURRENT_TIMESTAMP,NULL,'Y','N','N',NULL,NULL,NULL,NULL);

REPLACE INTO mysql.db VALUES
('localhost','sys','mysql.sys','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','N','Y');

REPLACE INTO mysql.tables_priv VALUES
('localhost','sys','mysql.sys','sys_config','root@localhost',
CURRENT_TIMESTAMP, 'Select', '');

