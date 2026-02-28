/* Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "udf_metadata_imp.h"
#include <mysql/components/service_implementation.h>
#include "m_ctype.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/sql_udf.h"
#include "template_utils.h"

namespace consts {
const std::string collation("collation");
const std::string charset("charset");
}  // namespace consts

void mysql_comp_udf_extension_init() { return; }

DEFINE_BOOL_METHOD(mysql_udf_metadata_imp::argument_set,
                   (UDF_ARGS * udf_args, const char *extension_type,
                    unsigned int index, void *in_value)) {
  DBUG_ASSERT(udf_args && udf_args->extension && in_value &&
              index < udf_args->arg_count);

  if (*(udf_args->arg_type) != Item_result::STRING_RESULT) {
    my_error(ER_DA_UDF_INVALID_ARGUMENT_TO_SET_CHARSET, MYF(0));
    return true;
  }
  auto *char_set_name = pointer_cast<const char *>(in_value);
  auto *x = pointer_cast<Udf_args_extension *>(udf_args->extension);
  if (!my_strcasecmp(system_charset_info, consts::charset.c_str(),
                     extension_type)) {
    x->charset_info[index] =
        get_charset_by_csname(char_set_name, MY_CS_PRIMARY, MYF(0));
    if (x->charset_info[index] == nullptr) {
      my_error(ER_DA_UDF_INVALID_CHARSET, MYF(0), char_set_name);
      return true;
    }
  } else if (!my_strcasecmp(system_charset_info, consts::collation.c_str(),
                            extension_type)) {
    x->charset_info[index] = get_charset_by_name(char_set_name, MYF(0));
    if (x->charset_info[index] == nullptr) {
      my_error(ER_DA_UDF_INVALID_COLLATION, MYF(0), char_set_name);
      return true;
    }
  } else {
    my_error(ER_DA_UDF_INVALID_EXTENSION_ARGUMENT_TYPE, MYF(0), extension_type);
    return true;
  }
  return false;
}

DEFINE_BOOL_METHOD(mysql_udf_metadata_imp::result_set,
                   (UDF_INIT * udf_init, const char *extension_type,
                    void *in_value)) {
  DBUG_ASSERT(udf_init && udf_init->extension && in_value);
  auto *x = pointer_cast<Udf_return_value_extension *>(udf_init->extension);
  if (x->result_type != Item_result::STRING_RESULT) {
    my_error(ER_DA_UDF_INVALID_RETURN_TYPE_TO_SET_CHARSET, MYF(0));
    return true;
  }
  auto *char_set_name = pointer_cast<char *>(in_value);
  if (!my_strcasecmp(system_charset_info, consts::charset.c_str(),
                     extension_type)) {
    x->charset_info =
        get_charset_by_csname(char_set_name, MY_CS_PRIMARY, MYF(0));

    if (x->charset_info == nullptr) {
      my_error(ER_DA_UDF_INVALID_CHARSET, MYF(0), char_set_name);
      return true;
    }
  } else if (!my_strcasecmp(system_charset_info, consts::collation.c_str(),
                            extension_type)) {
    x->charset_info = get_charset_by_name(char_set_name, MYF(0));
    if (x->charset_info == nullptr) {
      my_error(ER_DA_UDF_INVALID_COLLATION, MYF(0), char_set_name);
      return true;
    }
  } else {
    my_error(ER_DA_UDF_INVALID_EXTENSION_ARGUMENT_TYPE, MYF(0), extension_type);
    return true;
  }
  return false;
}

DEFINE_BOOL_METHOD(mysql_udf_metadata_imp::argument_get,
                   (UDF_ARGS * udf_args, const char *extension_type,
                    unsigned int index, void **out_value)) {
  DBUG_ASSERT(udf_args && udf_args->extension && index < udf_args->arg_count);
  auto *x = pointer_cast<Udf_args_extension *>(udf_args->extension);
  char *csname = nullptr;
  if (!my_strcasecmp(system_charset_info, consts::charset.c_str(),
                     extension_type)) {
    csname = const_cast<char *>(x->charset_info[index]->csname);
    *out_value = pointer_cast<void *>(csname);
  } else if (!my_strcasecmp(system_charset_info, consts::collation.c_str(),
                            extension_type)) {
    csname = const_cast<char *>(x->charset_info[index]->name);
    *out_value = pointer_cast<void *>(csname);
  } else {
    my_error(ER_DA_UDF_INVALID_EXTENSION_ARGUMENT_TYPE, MYF(0), extension_type);
    return true;
  }
  return false;
}

DEFINE_BOOL_METHOD(mysql_udf_metadata_imp::result_get,
                   (UDF_INIT * udf_init, const char *extension_type,
                    void **out_value)) {
  DBUG_ASSERT(udf_init && udf_init->extension && extension_type);
  auto *x = pointer_cast<Udf_return_value_extension *>(udf_init->extension);
  char *csname = nullptr;
  if (!my_strcasecmp(system_charset_info, consts::charset.c_str(),
                     extension_type)) {
    csname = const_cast<char *>(x->charset_info->csname);
    *out_value = pointer_cast<void *>(csname);
  } else if (!my_strcasecmp(system_charset_info, consts::collation.c_str(),
                            extension_type)) {
    csname = const_cast<char *>(x->charset_info->name);
    *out_value = pointer_cast<void *>(csname);
  } else {
    my_error(ER_DA_UDF_INVALID_EXTENSION_ARGUMENT_TYPE, MYF(0), extension_type);
    return true;
  }
  return false;
}
