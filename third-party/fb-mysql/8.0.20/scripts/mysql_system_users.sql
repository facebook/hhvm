-- Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License, version 2.0,
-- as published by the Free Software Foundation.
--
-- This program is also distributed with certain software (including
-- but not limited to OpenSSL) that is licensed under separate terms,
-- as designated in a particular file or component or in included license
-- documentation.  The authors of MySQL hereby grant you an additional
-- permission to link the program and your derivative works with the
-- separately licensed software that they have included with MySQL.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License, version 2.0, for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

--
-- Create the system users and grant them appropiate privileges.
-- These users are used for internally.
-- This script is called only while initializing the database.
--

-- Create an user that is used by plugins.
CREATE USER 'mysql.session'@localhost IDENTIFIED WITH caching_sha2_password
 AS '$A$005$THISISACOMBINATIONOFINVALIDSALTANDPASSWORDTHATMUSTNEVERBRBEUSED'
 ACCOUNT LOCK;
REVOKE ALL PRIVILEGES, GRANT OPTION FROM 'mysql.session'@localhost;
GRANT SELECT ON mysql.user TO 'mysql.session'@localhost;
GRANT SELECT ON `performance_schema`.* TO 'mysql.session'@localhost;
GRANT SUPER ON *.* TO 'mysql.session'@localhost;
GRANT SYSTEM_VARIABLES_ADMIN ON *.* TO 'mysql.session'@localhost;
GRANT SESSION_VARIABLES_ADMIN ON *.* TO 'mysql.session'@localhost;
GRANT PERSIST_RO_VARIABLES_ADMIN ON *.* TO 'mysql.session'@localhost;
GRANT CLONE_ADMIN ON *.* TO 'mysql.session'@localhost;
GRANT BACKUP_ADMIN ON *.* TO 'mysql.session'@localhost;
GRANT SHUTDOWN ON *.* TO 'mysql.session'@localhost;
GRANT CONNECTION_ADMIN ON *.* TO 'mysql.session'@localhost;
GRANT SYSTEM_USER ON *.* TO 'mysql.session'@localhost;

-- Create an user that is definer for information_schema view
CREATE USER 'mysql.infoschema'@localhost IDENTIFIED WITH caching_sha2_password
 AS '$A$005$THISISACOMBINATIONOFINVALIDSALTANDPASSWORDTHATMUSTNEVERBRBEUSED'
 ACCOUNT LOCK;
REVOKE ALL PRIVILEGES, GRANT OPTION FROM 'mysql.infoschema'@localhost;
GRANT SELECT ON *.* TO 'mysql.infoschema'@localhost;
