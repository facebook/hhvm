/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "auth_acls.h"

namespace consts {
/** Name of the static privileges */
const std::string SELECT("SELECT");
const std::string INSERT("INSERT");
const std::string UPDATE("UPDATE");
const std::string DELETE("DELETE");
const std::string CREATE("CREATE");
const std::string DROP("DROP");
const std::string RELOAD("RELOAD");
const std::string SHUTDOWN("SHUTDOWN");
const std::string PROCESS("PROCESS");
const std::string FILE("FILE");
const std::string GRANT("GRANT");
const std::string REFERENCES("REFERENCES");
const std::string INDEX("INDEX");
const std::string ALTER("ALTER");
const std::string SHOW_DATABASES("SHOW DATABASES");
const std::string SUPER("SUPER");
const std::string CREATE_TEMPORARY_TABLES("CREATE TEMPORARY TABLES");
const std::string LOCK_TABLES("LOCK TABLES");
const std::string EXECUTE("EXECUTE");
const std::string REPLICATION_SLAVE("REPLICATION SLAVE");
const std::string REPLICATION_CLIENT("REPLICATION CLIENT");
const std::string CREATE_VIEW("CREATE VIEW");
const std::string SHOW_VIEW("SHOW VIEW");
const std::string CREATE_ROUTINE("CREATE ROUTINE");
const std::string ALTER_ROUTINE("ALTER ROUTINE");
const std::string CREATE_USER("CREATE USER");
const std::string EVENT("EVENT");
const std::string TRIGGER("TRIGGER");
const std::string CREATE_TABLESPACE("CREATE TABLESPACE");
const std::string CREATE_ROLE("CREATE ROLE");
const std::string DROP_ROLE("DROP ROLE");
}  // namespace consts

/** Consts for static privileges */
const std::vector<std::string> global_acls_vector = {
    consts::SELECT,
    consts::INSERT,
    consts::UPDATE,
    consts::DELETE,
    consts::CREATE,
    consts::DROP,
    consts::RELOAD,
    consts::SHUTDOWN,
    consts::PROCESS,
    consts::FILE,
    consts::GRANT,
    consts::REFERENCES,
    consts::INDEX,
    consts::ALTER,
    consts::SHOW_DATABASES,
    consts::SUPER,
    consts::CREATE_TEMPORARY_TABLES,
    consts::LOCK_TABLES,
    consts::EXECUTE,
    consts::REPLICATION_SLAVE,
    consts::REPLICATION_CLIENT,
    consts::CREATE_VIEW,
    consts::SHOW_VIEW,
    consts::CREATE_ROUTINE,
    consts::ALTER_ROUTINE,
    consts::CREATE_USER,
    consts::EVENT,
    consts::TRIGGER,
    consts::CREATE_TABLESPACE,
    consts::CREATE_ROLE,
    consts::DROP_ROLE};

/** Bitmap offsets for static privileges */
const std::unordered_map<std::string, int> global_acls_map{
    {consts::SELECT, 0},
    {consts::INSERT, 1},
    {consts::UPDATE, 2},
    {consts::DELETE, 3},
    {consts::CREATE, 4},
    {consts::DROP, 5},
    {consts::RELOAD, 6},
    {consts::SHUTDOWN, 7},
    {consts::PROCESS, 8},
    {consts::FILE, 9},
    {consts::GRANT, 10},
    {consts::REFERENCES, 11},
    {consts::INDEX, 12},
    {consts::ALTER, 13},
    {consts::SHOW_DATABASES, 14},
    {consts::SUPER, 15},
    {consts::CREATE_TEMPORARY_TABLES, 16},
    {consts::LOCK_TABLES, 17},
    {consts::EXECUTE, 18},
    {consts::REPLICATION_SLAVE, 19},
    {consts::REPLICATION_CLIENT, 20},
    {consts::CREATE_VIEW, 21},
    {consts::SHOW_VIEW, 22},
    {consts::CREATE_ROUTINE, 23},
    {consts::ALTER_ROUTINE, 24},
    {consts::CREATE_USER, 25},
    {consts::EVENT, 26},
    {consts::TRIGGER, 27},
    {consts::CREATE_TABLESPACE, 28},
    {consts::CREATE_ROLE, 29},
    {consts::DROP_ROLE, 30}};
