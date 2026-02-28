/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/applicable_roles.h"

namespace dd {
namespace system_views {

const String_type &Applicable_roles::cte_expression() {
  static String_type s_cte_expression(
      "WITH RECURSIVE role_graph (c_parent_user, c_parent_host,"
      "                                 c_from_user, c_from_host,"
      "                                 c_to_user, c_to_host, role_path,"
      "                                 c_with_admin, c_enabled) AS "

      // Get current user as seed 1 for CTE
      "  ((SELECT "
      "      INTERNAL_GET_USERNAME(), INTERNAL_GET_HOSTNAME(), "
      "      CONVERT(INTERNAL_GET_USERNAME() USING utf8mb4), "
      "      CONVERT(INTERNAL_GET_HOSTNAME() USING utf8mb4), "
      "      CAST('' as CHAR(64) CHARSET utf8mb4), "
      "      CAST('' as CHAR(255) CHARSET utf8mb4), "
      "      CAST(SHA2(CONCAT(QUOTE(INTERNAL_GET_USERNAME()),'@', "
      "                       QUOTE(INTERNAL_GET_HOSTNAME())), 256) "
      /*
        The use of CHAR(17000) enables use of MEDIUMTEXT datatype for
        'role_path' column.

        The CAST function uses value 'size' in CHAR(<size>) to determine
        the resulting type of CAST() function. We use size as 17000 to
        enable optimizer to pick MEDIUM TEXT. e.g.,

        CREATE VIEW v1 AS WITH RECURSIVE role1 (c1, c2) as
        (SELECT CAST('' as CHAR(16383) CHARSET utf8mb4),
                CAST('' as CHAR(16400) CHARSET utf8mb4))
        SELECT * FROM role1 WHERE c1 != '';
        SHOW COLUMNS FROM v1;
        Field Type       Null  Key  Default Extra
        c1    text       YES        NULL
        c2    mediumtext YES        NULL

        Using TEXT enable about ~2k SHA2 values to be stored in
        'role_path'. And using MEDIUMTEXT enables us to hold about ~500k
        SHA2 values.
      */
      "           AS CHAR(17000) CHARSET utf8mb4), "
      "      CAST('N' as CHAR(1) CHARSET utf8mb4), "
      "      FALSE "

      /*
         Fetch all mandatory role names which are not be assigned to
         current user, as seed 2 for CTE
      */
      "    UNION "
      "      SELECT "
      "        INTERNAL_GET_USERNAME(), INTERNAL_GET_HOSTNAME(), "
      "        CONVERT(ROLE_NAME USING utf8mb4), "
      "        CONVERT(ROLE_HOST USING utf8mb4), "
      "        INTERNAL_GET_USERNAME(), INTERNAL_GET_HOSTNAME(), "
      "        CAST(SHA2(CONCAT(QUOTE(ROLE_NAME),'@', "
      "                  CONVERT(QUOTE(ROLE_HOST) using utf8mb4)), 256) "
      "             AS CHAR(17000) CHARSET utf8mb4), "
      "        CAST('N' as CHAR(1) CHARSET utf8mb4), "
      "        FALSE "
      "      FROM JSON_TABLE(INTERNAL_GET_MANDATORY_ROLES_JSON(),"
      "            '$[*]' COLUMNS ("
      "            ROLE_NAME VARCHAR(255) CHARSET utf8mb4 PATH '$.ROLE_NAME', "
      "            ROLE_HOST VARCHAR(255) CHARSET utf8mb4 PATH '$.ROLE_HOST') "
      "            ) mandatory_roles "
      "      WHERE CONCAT(QUOTE(ROLE_NAME),'@', "
      "                   CONVERT(QUOTE(ROLE_HOST) using utf8mb4)) NOT IN "
      "            (SELECT CONCAT(QUOTE(FROM_USER),'@', "
      "                       CONVERT(QUOTE(FROM_HOST) using utf8mb4)) "
      "             FROM mysql.role_edges "
      "             WHERE TO_USER = INTERNAL_GET_USERNAME() AND "
      "             CONVERT(TO_HOST using utf8mb4) = INTERNAL_GET_HOSTNAME())"
      "   ) "

      // Recursive CTE SELECT query
      "  UNION "
      "    SELECT c_parent_user, c_parent_host, "
      "      CONVERT(FROM_USER USING utf8mb4), "
      "      CONVERT(FROM_HOST USING utf8mb4), "
      "      TO_USER, TO_HOST, "
      /*
        Pass NULL for role_path to stop recursive CTE execution, once
        we locate a role that is already in the currently visited
        role_path.
      */
      "      IF(LOCATE(SHA2(CONCAT(QUOTE(FROM_USER),'@', "
      "                     CONVERT(QUOTE(FROM_HOST) using utf8mb4)), 256), "
      "                role_path) = 0, "
      "         CONCAT(role_path,'->', SHA2(CONCAT(QUOTE(FROM_USER),'@',"
      "           CONVERT(QUOTE(FROM_HOST) using utf8mb4)), 256)), NULL), "
      "      WITH_ADMIN_OPTION, "
      "      IF(c_enabled OR "
      "       INTERNAL_IS_ENABLED_ROLE(FROM_USER, FROM_HOST), TRUE, FALSE) "
      "    FROM mysql.role_edges, role_graph "

      "    WHERE TO_USER = c_from_user AND "
      "          CONVERT(TO_HOST using utf8mb4)= c_from_host AND "
      "          role_path IS NOT NULL)");
  return s_cte_expression;
}

const Applicable_roles &Applicable_roles::instance() {
  static Applicable_roles *s_instance = new Applicable_roles();
  return *s_instance;
}

Applicable_roles::Applicable_roles() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_distinct();

  m_target_def.add_cte_expression(cte_expression());

  m_target_def.add_field(FIELD_USER, "USER", "c_parent_user");
  m_target_def.add_field(FIELD_HOST, "HOST", "c_parent_host");
  m_target_def.add_field(FIELD_GRANTEE, "GRANTEE", "c_to_user");
  m_target_def.add_field(FIELD_GRANTEE_HOST, "GRANTEE_HOST", "c_to_host");
  m_target_def.add_field(FIELD_ROLE_NAME, "ROLE_NAME", "c_from_user");
  m_target_def.add_field(FIELD_ROLE_HOST, "ROLE_HOST", "c_from_host");
  m_target_def.add_field(FIELD_IS_GRANTABLE, "IS_GRANTABLE",
                         "IF(c_with_admin = 'N', 'NO', 'YES')");
  m_target_def.add_field(
      FIELD_IS_DEFAULT, "IS_DEFAULT",
      " (SELECT IF(COUNT(*), 'YES', 'NO') "
      "   FROM mysql.default_roles "
      "   WHERE DEFAULT_ROLE_USER = c_from_user AND "
      "         CONVERT(DEFAULT_ROLE_HOST using utf8mb4) = c_from_host AND "
      "         USER = c_parent_user AND "
      "         CONVERT(HOST using utf8mb4) = c_parent_host) ");
  m_target_def.add_field(
      FIELD_IS_MANDATORY, "IS_MANDATORY",
      "IF(INTERNAL_IS_MANDATORY_ROLE(c_from_user, c_from_host), 'YES', 'NO') ");

  m_target_def.add_from("role_graph");

  // Filter out the seed row that represent current user from the output.
  m_target_def.add_where("c_to_user != ''");
}

}  // namespace system_views
}  // namespace dd
