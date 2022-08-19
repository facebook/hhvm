/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_write_set_handler.h"

#include <inttypes.h>
#include <stddef.h>
#include <sys/types.h>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../extra/lz4/my_xxhash.h"  // IWYU pragma: keep
#include "lex_string.h"
#include "m_ctype.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_murmur3.h"  // murmur3_32
#include "mysql_com.h"
#include "sql/field.h"  // Field
#include "sql/json_binary.h"
#include "sql/json_dom.h"
#include "sql/key.h"
#include "sql/query_options.h"
#include "sql/rpl_transaction_write_set_ctx.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/system_variables.h"
#include "sql/table.h"  // TABLE
#include "sql/transaction_info.h"
#include "template_utils.h"

#define HASH_STRING_SEPARATOR "½"

const char *transaction_write_set_hashing_algorithms[] = {"OFF", "MURMUR32",
                                                          "XXHASH64", nullptr};

const char *get_write_set_algorithm_string(unsigned int algorithm) {
  switch (algorithm) {
    case HASH_ALGORITHM_OFF:
      return "OFF";
    case HASH_ALGORITHM_MURMUR32:
      return "MURMUR32";
    case HASH_ALGORITHM_XXHASH64:
      return "XXHASH64";
    default:
      return "UNKNOWN ALGORITHM";
  }
}

template <class type>
uint64 calc_hash(ulong algorithm, type T, size_t len) {
  if (algorithm == HASH_ALGORITHM_MURMUR32)
    return (murmur3_32((const uchar *)T, len, 0));
  else
    return (MY_XXH64((const uchar *)T, len, 0));
}

/**
  Function to check if the given TABLE has any foreign key field. This is
  needed to be checked to get the hash of the field value in the foreign
  table.

  This function is meant to be only called by add_pke() function, some
  conditions are check there for performance optimization.

  @param[in] table - TABLE object
  @param[in] thd - THD object pointing to current thread.

  @param[out] foreign_key_map - a standard map which keeps track of the
                                foreign key fields.
*/
static void check_foreign_key(
    TABLE *table,
#ifndef DBUG_OFF
    THD *thd,
#endif
    std::map<std::string, std::string> &foreign_key_map) {
  DBUG_TRACE;
  DBUG_ASSERT(!(thd->variables.option_bits & OPTION_NO_FOREIGN_KEY_CHECKS));
  DBUG_ASSERT(table->s->foreign_keys > 0);

  TABLE_SHARE_FOREIGN_KEY_INFO *fk = table->s->foreign_key;
  std::string pke_prefix;
  pke_prefix.reserve(NAME_LEN * 5);

  for (uint i = 0; i < table->s->foreign_keys; i++) {
    /*
      There are two situations on which there is no
      unique_constraint_name, which means that the foreign key
      must be skipped.

      1) The referenced table was dropped using
         foreign_key_checks= 0, on that case we cannot check
         foreign key and need to skip it.

      2) The foreign key does reference a non unique key, thence
         it must be skipped since it cannot be used to check
         conflicts/dependencies.

         Example:
           CREATE TABLE t1 (c1 INT PRIMARY KEY, c2 INT, KEY(c2));
           CREATE TABLE t2 (x1 INT PRIMARY KEY, x2 INT,
                            FOREIGN KEY (x2) REFERENCES t1(c2));

           DELETE FROM t1 WHERE c1=1;
             does generate the PKEs:
               PRIMARY½test½4t1½21½1

           INSERT INTO t2 VALUES (1,1);
             does generate the PKEs:
               PRIMARY½test½4t2½21½1

           which does not contain PKE for the non unique key c2.
    */
    if (0 == fk[i].unique_constraint_name.length) continue;

    const std::string referenced_schema_name_length =
        std::to_string(fk[i].referenced_table_db.length);
    const std::string referenced_table_name_length =
        std::to_string(fk[i].referenced_table_name.length);

    /*
      Prefix the hash keys with the referenced index name.
    */
    pke_prefix.clear();
    pke_prefix.append(fk[i].unique_constraint_name.str,
                      fk[i].unique_constraint_name.length);
    pke_prefix.append(HASH_STRING_SEPARATOR);
    pke_prefix.append(fk[i].referenced_table_db.str,
                      fk[i].referenced_table_db.length);
    pke_prefix.append(HASH_STRING_SEPARATOR);
    pke_prefix.append(referenced_schema_name_length);
    pke_prefix.append(fk[i].referenced_table_name.str,
                      fk[i].referenced_table_name.length);
    pke_prefix.append(HASH_STRING_SEPARATOR);
    pke_prefix.append(referenced_table_name_length);

    /*
      Foreign key must not have a empty column list.
    */
    DBUG_ASSERT(fk[i].columns > 0);
    for (uint c = 0; c < fk[i].columns; c++)
      foreign_key_map[fk[i].column_name[c].str] = pke_prefix;
  }
}

#ifndef DBUG_OFF
static void debug_check_for_write_sets(
    std::vector<std::string> &key_list_to_hash,
    std::vector<uint64> &hash_list) {
  DBUG_EXECUTE_IF(
      "PKE_assert_single_primary_key_generated_insert",
      DBUG_ASSERT(key_list_to_hash.size() == 1);
      DBUG_ASSERT(key_list_to_hash[0] ==
                  "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
                  "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 1);
      DBUG_ASSERT(hash_list[0] == 340395741608101502ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_single_primary_key_generated_update",
      DBUG_ASSERT(key_list_to_hash.size() == 1);
      DBUG_ASSERT(
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR "1" ||
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 1);
      DBUG_ASSERT(hash_list[0] == 8563267070232261320ULL ||
                  hash_list[0] == 340395741608101502ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_multi_primary_key_generated_insert",
      DBUG_ASSERT(key_list_to_hash.size() == 1);
      DBUG_ASSERT(key_list_to_hash[0] ==
                  "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
                  "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR
                  "12" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 1);
      DBUG_ASSERT(hash_list[0] == 13655149628280894901ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_multi_primary_key_generated_update",
      DBUG_ASSERT(key_list_to_hash.size() == 1);
      DBUG_ASSERT(
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR
              "12" HASH_STRING_SEPARATOR "1" ||
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR
              "12" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 1);
      DBUG_ASSERT(hash_list[0] == 16833405476607614310ULL ||
                  hash_list[0] == 13655149628280894901ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_single_primary_unique_key_generated_insert",
      DBUG_ASSERT(key_list_to_hash.size() == 3);
      DBUG_ASSERT(
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[1] ==
              "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "22" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[2] ==
              "c3" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 3);
      DBUG_ASSERT(hash_list[0] == 340395741608101502ULL &&
                  hash_list[1] == 12796928550717161120ULL &&
                  hash_list[2] == 11199547876733116431ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_single_primary_unique_key_generated_update",
      DBUG_ASSERT(key_list_to_hash.size() == 3);
      DBUG_ASSERT(
          (key_list_to_hash[0] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "25" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[1] ==
               "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "22" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[2] ==
               "c3" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR "1") ||
          (key_list_to_hash[0] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[1] ==
               "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "22" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[2] ==
               "c3" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR "1"));
      DBUG_ASSERT(hash_list.size() == 3);
      DBUG_ASSERT((hash_list[0] == 7803002059431370747ULL &&
                   hash_list[1] == 12796928550717161120ULL &&
                   hash_list[2] == 11199547876733116431ULL) ||
                  (hash_list[0] == 340395741608101502ULL &&
                   hash_list[1] == 12796928550717161120ULL &&
                   hash_list[2] == 11199547876733116431ULL)););

  DBUG_EXECUTE_IF(
      "PKE_assert_multi_primary_unique_key_generated_insert",
      DBUG_ASSERT(key_list_to_hash.size() == 3);
      DBUG_ASSERT(
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR
              "12" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[1] ==
              "b" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[2] ==
              "c" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "24" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 3);
      DBUG_ASSERT(hash_list[0] == 13655149628280894901ULL &&
                  hash_list[1] == 8954424140835647185ULL &&
                  hash_list[2] == 3520344117573337805ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_multi_primary_unique_key_generated_update",
      DBUG_ASSERT(key_list_to_hash.size() == 3);
      DBUG_ASSERT(
          (key_list_to_hash[0] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR
               "12" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[1] ==
               "b" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[2] ==
               "c" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "24" HASH_STRING_SEPARATOR "1") ||
          (key_list_to_hash[0] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "25" HASH_STRING_SEPARATOR
               "12" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[1] ==
               "b" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[2] ==
               "c" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "24" HASH_STRING_SEPARATOR "1"));
      DBUG_ASSERT(hash_list.size() == 3);
      DBUG_ASSERT((hash_list[0] == 13655149628280894901ULL &&
                   hash_list[1] == 8954424140835647185ULL &&
                   hash_list[2] == 3520344117573337805ULL) ||
                  (hash_list[0] == 17122769277112661326ULL &&
                   hash_list[1] == 8954424140835647185ULL &&
                   hash_list[2] == 3520344117573337805ULL)););

  DBUG_EXECUTE_IF(
      "PKE_assert_multi_foreign_key_generated_insert",
      DBUG_ASSERT(key_list_to_hash.size() == 4);
      DBUG_ASSERT(
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t3" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR
              "15" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[1] ==
              "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t3" HASH_STRING_SEPARATOR "25" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[2] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[3] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t2" HASH_STRING_SEPARATOR "25" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 4);
      DBUG_ASSERT(hash_list[0] == 3283233640848908273ULL &&
                  hash_list[1] == 17221725733811443497ULL &&
                  hash_list[2] == 340395741608101502ULL &&
                  hash_list[3] == 14682037479339770823ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_multi_foreign_key_generated_update",
      DBUG_ASSERT(key_list_to_hash.size() == 4);
      DBUG_ASSERT(
          (key_list_to_hash[0] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t3" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR
               "15" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[1] ==
               "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t3" HASH_STRING_SEPARATOR "25" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[2] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[3] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t2" HASH_STRING_SEPARATOR "25" HASH_STRING_SEPARATOR "1") ||
          (key_list_to_hash[0] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t3" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR
               "15" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[1] ==
               "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t3" HASH_STRING_SEPARATOR "25" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[2] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "23" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[3] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t2" HASH_STRING_SEPARATOR "25" HASH_STRING_SEPARATOR "1"));
      DBUG_ASSERT(hash_list.size() == 4);
      DBUG_ASSERT((hash_list[0] == 3283233640848908273ULL &&
                   hash_list[1] == 17221725733811443497ULL &&
                   hash_list[2] == 340395741608101502ULL &&
                   hash_list[3] == 14682037479339770823ULL) ||
                  (hash_list[0] == 12666755939597291234ULL &&
                   hash_list[1] == 17221725733811443497ULL &&
                   hash_list[2] == 8563267070232261320ULL &&
                   hash_list[3] == 14682037479339770823ULL)););

  DBUG_EXECUTE_IF(
      "PKE_assert_foreign_key_on_referenced_unique_key_parent_generated_insert",
      DBUG_ASSERT(key_list_to_hash.size() == 2);
      DBUG_ASSERT(
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "22" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[1] ==
              "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "22" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 2);
      DBUG_ASSERT(hash_list[0] == 16097759999475440183ULL &&
                  hash_list[1] == 12796928550717161120ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_foreign_key_on_referenced_unique_key_generated_insert",
      DBUG_ASSERT(key_list_to_hash.size() == 2);
      DBUG_ASSERT(
          key_list_to_hash[0] ==
              "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t2" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1" &&
          key_list_to_hash[1] ==
              "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
              "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 2);
      DBUG_ASSERT(hash_list[0] == 10002085147685770725ULL &&
                  hash_list[1] == 8692935619695688993ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_foreign_key_on_referenced_unique_key_generated_update",
      DBUG_ASSERT(key_list_to_hash.size() == 2);
      DBUG_ASSERT(
          (key_list_to_hash[0] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t2" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[1] ==
               "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "22" HASH_STRING_SEPARATOR "1") ||
          (key_list_to_hash[0] ==
               "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t2" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1" &&
           key_list_to_hash[1] ==
               "c2" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
               "4t1" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1"));
      DBUG_ASSERT(hash_list.size() == 2);
      DBUG_ASSERT((hash_list[0] == 10002085147685770725ULL &&
                   hash_list[1] == 12796928550717161120ULL) ||
                  (hash_list[0] == 10002085147685770725ULL &&
                   hash_list[1] == 8692935619695688993ULL)););

  DBUG_EXECUTE_IF(
      "PKE_assert_foreign_key_on_referenced_non_unique_key_parent_generated_"
      "insert",
      DBUG_ASSERT(key_list_to_hash.size() == 1);
      DBUG_ASSERT(key_list_to_hash[0] ==
                  "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
                  "4t1" HASH_STRING_SEPARATOR "22" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 1);
      DBUG_ASSERT(hash_list[0] == 16097759999475440183ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_foreign_key_on_referenced_non_unique_key_generated_insert",
      DBUG_ASSERT(key_list_to_hash.size() == 1);
      DBUG_ASSERT(key_list_to_hash[0] ==
                  "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
                  "4t2" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 1);
      DBUG_ASSERT(hash_list[0] == 10002085147685770725ULL););

  DBUG_EXECUTE_IF(
      "PKE_assert_foreign_key_on_referenced_non_unique_key_generated_update",
      DBUG_ASSERT(key_list_to_hash.size() == 1);
      DBUG_ASSERT(key_list_to_hash[0] ==
                  "PRIMARY" HASH_STRING_SEPARATOR "test" HASH_STRING_SEPARATOR
                  "4t2" HASH_STRING_SEPARATOR "21" HASH_STRING_SEPARATOR "1");
      DBUG_ASSERT(hash_list.size() == 1);
      DBUG_ASSERT(hash_list[0] == 10002085147685770725ULL););
}
#endif

/**
  Function to generate the hash of the string passed to this function.

  @param[in] pke - the string to be hashed.
  @param[in] thd - THD object pointing to current thread.
  @param[in] write_sets - list of all write sets
  @param[in] hash_list - list of all hashes
*/

static void generate_hash_pke(const std::string &pke, THD *thd
#ifndef DBUG_OFF
                              ,
                              std::vector<std::string> &write_sets,
                              std::vector<uint64> &hash_list
#endif
) {
  DBUG_TRACE;
  DBUG_ASSERT(thd->variables.transaction_write_set_extraction !=
              HASH_ALGORITHM_OFF);

  uint64 hash = calc_hash<const char *>(
      thd->variables.transaction_write_set_extraction, pke.c_str(), pke.size());
  thd->get_transaction()->get_transaction_write_set_ctx()->add_write_set(hash);

#ifndef DBUG_OFF
  write_sets.push_back(pke);
  hash_list.push_back(hash);
#endif
  DBUG_PRINT("info", ("pke: %s; hash: %" PRIu64, pke.c_str(), hash));
}

/**
  Function to generate set of hashes for a multi-valued key

  @param[in] prefix_pke  - stringified non-multi-valued prefix of key
  @param[in] thd         - THD object pointing to current thread.
  @param[in] fld         - multi-valued keypart's field
  @param[in] write_sets  - DEBUG ONLY, vector of added PKEs
  @param[in] hash_list   - DEBUG ONLY, list of all hashes
*/

static void generate_mv_hash_pke(const std::string &prefix_pke, THD *thd,
                                 Field *fld
#ifndef DBUG_OFF
                                 ,
                                 std::vector<std::string> &write_sets,
                                 std::vector<uint64> &hash_list
#endif
) {
  Field_typed_array *field = down_cast<Field_typed_array *>(fld);

  json_binary::Value v(
      json_binary::parse_binary(field->get_binary(), field->data_length()));
  uint elems = v.element_count();
  if (!elems || field->is_null()) {
    // Multi-valued key part doesn't contain actual values.
    // No need to hash prefix pke as it won't cause conflicts.
  } else {
    const CHARSET_INFO *cs = field->charset();
    int max_length = cs->coll->strnxfrmlen(cs, field->key_length());
    std::unique_ptr<uchar[]> pk_value(new uchar[max_length + 1]());
    DBUG_ASSERT(v.type() == json_binary::Value::ARRAY);

    for (uint i = 0; i < elems; i++) {
      std::string pke = prefix_pke;
      json_binary::Value elt = v.element(i);
      Json_wrapper wr(elt);
      /*
        convert to normalized string and store so that it can be
        sorted using binary comparison functions like memcmp.
      */
      size_t length = field->make_sort_key(&wr, pk_value.get(), max_length);
      pk_value[length] = 0;

      pke.append(pointer_cast<char *>(pk_value.get()), length);
      pke.append(HASH_STRING_SEPARATOR);
      pke.append(std::to_string(length));
      generate_hash_pke(pke, thd
#ifndef DBUG_OFF
                        ,
                        write_sets, hash_list
#endif
      );
    }
  }
}

void add_pke(TABLE *table, THD *thd, uchar *record) {
  DBUG_TRACE;
  DBUG_ASSERT(record == table->record[0] || record == table->record[1]);
  /*
    The next section extracts the primary key equivalent of the rows that are
    changing during the current transaction.

    1. The primary key field is always stored in the key_part[0] so we can
    simply read the value from the table->s->keys.

    2. Along with primary key we also need to extract the unique key values to
       look for the places where we are breaking the unique key constraints.

    These keys (primary/unique) are prefixed with their index names.

    In MySQL, the name of a PRIMARY KEY is PRIMARY. For other indexes, if
    you do not assign a name, the index is assigned the same name as the
    first indexed column, with an optional suffix (_2, _3, ...) to make it
    unique.

    example :
       CREATE TABLE db1.t1 (i INT NOT NULL PRIMARY KEY, j INT UNIQUE KEY, k INT
                            UNIQUE KEY);

       INSERT INTO db1.t1 VALUES(1, 2, 3);

       Here the write set string will have three values and the prepared value
    before hash function is used will be :

       i -> PRIMARYdb13t1211 => PRIMARY is the index name (for primary key)

       j -> jdb13t1221       => 'j' is the index name (for first unique key)
       k -> kdb13t1231       => 'k' is the index name (for second unique key)

    Finally these value are hashed using the murmur hash function to prevent
    sending more for certification algorithm.
  */
  Rpl_transaction_write_set_ctx *ws_ctx =
      thd->get_transaction()->get_transaction_write_set_ctx();
  bool writeset_hashes_added = false;

  if (table->key_info && (table->s->primary_key < MAX_KEY)) {
    ptrdiff_t ptrdiff = record - table->record[0];
    std::string pke_schema_table;
    pke_schema_table.reserve(NAME_LEN * 3);
    pke_schema_table.append(HASH_STRING_SEPARATOR);
    pke_schema_table.append(table->s->db.str, table->s->db.length);
    pke_schema_table.append(HASH_STRING_SEPARATOR);
    pke_schema_table.append(std::to_string(table->s->db.length));
    pke_schema_table.append(table->s->table_name.str,
                            table->s->table_name.length);
    pke_schema_table.append(HASH_STRING_SEPARATOR);
    pke_schema_table.append(std::to_string(table->s->table_name.length));

    std::string pke;
    pke.reserve(NAME_LEN * 5);

#ifndef DBUG_OFF
    std::vector<std::string> write_sets;
    std::vector<uint64> hash_list;
#endif

    for (uint key_number = 0; key_number < table->s->keys; key_number++) {
      // Skip non unique.
      if (!((table->key_info[key_number].flags & (HA_NOSAME)) == HA_NOSAME))
        continue;

      pke.clear();
      pke.append(table->key_info[key_number].name);
      pke.append(pke_schema_table);

      uint i = 0;
      // Whether the key has mv keypart which have to be handled separately
      Field *mv_field = nullptr;
      for (/*empty*/; i < table->key_info[key_number].user_defined_key_parts;
           i++) {
        /* Get the primary key field index. */
        int index = table->key_info[key_number].key_part[i].fieldnr;
        Field *field = table->field[index - 1];

        /* Ignore if the value is NULL. */
        if (field->is_null(ptrdiff)) break;
        if (field->is_array()) {
          // There can be only one multi-valued key part per key
          DBUG_ASSERT(!mv_field);
          mv_field = field;
          // Skip it for now
          continue;
        }

        /*
          Update the field offset as we may be working on table->record[0]
          or table->record[1], depending on the "record" parameter.
         */
        field->move_field_offset(ptrdiff);
        const CHARSET_INFO *cs = field->charset();
        int max_length = cs->coll->strnxfrmlen(cs, field->pack_length());
        std::unique_ptr<uchar[]> pk_value(new uchar[max_length + 1]());

        /*
          convert to normalized string and store so that it can be
          sorted using binary comparison functions like memcmp.
        */
        size_t length = field->make_sort_key(pk_value.get(), max_length);
        pk_value[length] = 0;

        pke.append(pointer_cast<char *>(pk_value.get()), length);
        pke.append(HASH_STRING_SEPARATOR);
        pke.append(std::to_string(length));

        field->move_field_offset(-ptrdiff);
      }
      /*
        If any part of the key is NULL, ignore adding it to hash keys.
        NULL cannot conflict with any value.
        Eg: create table t1(i int primary key not null, j int, k int,
                                                unique key (j, k));
            insert into t1 values (1, 2, NULL);
            insert into t1 values (2, 2, NULL); => this is allowed.
      */
      if (i == table->key_info[key_number].user_defined_key_parts) {
        if (mv_field) {
          mv_field->move_field_offset(ptrdiff);
          generate_mv_hash_pke(pke, thd, mv_field
#ifndef DBUG_OFF
                               ,
                               write_sets, hash_list
#endif
          );
          mv_field->move_field_offset(-ptrdiff);
        } else {
          generate_hash_pke(pke, thd
#ifndef DBUG_OFF
                            ,
                            write_sets, hash_list
#endif
          );
        }
        writeset_hashes_added = true;
      } else {
        /* This is impossible to happen in case of primary keys */
        DBUG_ASSERT(key_number != 0);
      }
    }

    /*
      Foreign keys handling.
      We check the foreign keys existence here and not at check_foreign_key()
      function to avoid allocate foreign_key_map when it is not needed.

      OPTION_NO_FOREIGN_KEY_CHECKS bit in options_bits is set at two places

      1) If the user executed 'SET foreign_key_checks= 0' on the local session
      before executing the query.
      or
      2) We are applying a RBR event (i.e., the event is from a remote server)
      and logic in Rows_log_event::do_apply_event found out that the event is
      generated from a remote server session that disabled foreign_key_checks
      (using 'SET foreign_key_checks=0').

      In either of the above cases (i.e., the foreign key check is disabled for
      the current query/current event), we should ignore generating
      the foreign key information as they should not participate
      in the conflicts detecting algorithm.
    */
    if (!(thd->variables.option_bits & OPTION_NO_FOREIGN_KEY_CHECKS) &&
        table->s->foreign_keys > 0) {
      std::map<std::string, std::string> foreign_key_map;
      check_foreign_key(table,
#ifndef DBUG_OFF
                        thd,
#endif
                        foreign_key_map);

      if (!foreign_key_map.empty()) {
        for (uint i = 0; i < table->s->fields; i++) {
          Field *field = table->field[i];
          if (field->is_null(ptrdiff)) continue;
          /*
            Update the field offset, since we may be operating on
            table->record[0] or table->record[1] and both have
            different offsets.
          */
          field->move_field_offset(ptrdiff);
          std::map<std::string, std::string>::iterator it =
              foreign_key_map.find(field->field_name);
          if (foreign_key_map.end() != it) {
            std::string pke_prefix = it->second;

            const CHARSET_INFO *cs = field->charset();
            int max_length = cs->coll->strnxfrmlen(cs, field->pack_length());
            std::unique_ptr<uchar[]> pk_value(new uchar[max_length + 1]());

            /*
              convert to normalized string and store so that it can be
              sorted using binary comparison functions like memcmp.
            */
            size_t length = field->make_sort_key(pk_value.get(), max_length);
            pk_value[length] = 0;

            pke_prefix.append(pointer_cast<char *>(pk_value.get()), length);
            pke_prefix.append(HASH_STRING_SEPARATOR);
            pke_prefix.append(std::to_string(length));

            generate_hash_pke(pke_prefix, thd
#ifndef DBUG_OFF
                              ,
                              write_sets, hash_list
#endif
            );
            writeset_hashes_added = true;
          }
          /* revert the field object record offset back */
          field->move_field_offset(-ptrdiff);
        }
      }
    }

    if (table->s->foreign_key_parents > 0)
      ws_ctx->set_has_related_foreign_keys();

#ifndef DBUG_OFF
    debug_check_for_write_sets(write_sets, hash_list);
#endif
  }

  if (!writeset_hashes_added) ws_ctx->set_has_missing_keys();
}
