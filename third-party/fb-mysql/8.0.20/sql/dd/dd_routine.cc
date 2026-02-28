/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "sql/dd/dd_routine.h"  // Routine methods

#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <memory>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_time.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_table.h"                 // dd::get_new_field_type
#include "sql/dd/impl/utils.h"               // dd::my_time_t_to_ull_datetime
#include "sql/dd/properties.h"               // dd::Properties
#include "sql/dd/string_type.h"
#include "sql/dd/types/function.h"                // dd::Function
#include "sql/dd/types/parameter.h"               // dd::Parameter
#include "sql/dd/types/parameter_type_element.h"  // dd::Parameter_type_element
#include "sql/dd/types/procedure.h"               // dd::Procedure
#include "sql/dd/types/routine.h"
#include "sql/dd/types/schema.h"  // dd::Schema
#include "sql/dd/types/view.h"
#include "sql/field.h"
#include "sql/histograms/value_map.h"
#include "sql/item_create.h"
#include "sql/key.h"
#include "sql/sp.h"
#include "sql/sp_head.h"      // sp_head
#include "sql/sp_pcontext.h"  // sp_variable
#include "sql/sql_class.h"
#include "sql/sql_connect.h"
#include "sql/sql_db.h"  // get_default_db_collation
#include "sql/sql_lex.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/tztime.h"  // Time_zone
#include "sql_string.h"
#include "typelib.h"

namespace dd {

////////////////////////////////////////////////////////////////////////////////

/**
  Helper method for create_routine() to fill return type information of stored
  routine from the sp_head.
  from the sp_head.

  @param[in]  thd        Thread handle.
  @param[in]  sp         Stored routine object.
  @param[out] sf         dd::Function object.
*/

static void fill_dd_function_return_type(THD *thd, sp_head *sp, Function *sf) {
  DBUG_TRACE;

  Create_field *return_field = &sp->m_return_field_def;
  DBUG_ASSERT(return_field != nullptr);

  // Set result data type.
  sf->set_result_data_type(get_new_field_type(return_field->sql_type));

  // We need a fake table and share to generate a utf8 string
  // representation of result data type.
  TABLE table;
  TABLE_SHARE share;
  table.s = &share;
  table.in_use = thd;
  table.s->db_low_byte_first = true;

  // Reset result data type in utf8
  sf->set_result_data_type_utf8(
      get_sql_type_by_create_field(&table, *return_field));

  // Set result is_zerofill flag.
  sf->set_result_zerofill(return_field->is_zerofill);

  // Set result is_unsigned flag.
  sf->set_result_unsigned(return_field->is_unsigned);

  /*
    set result char length.
    Note that setting this only affects information schema views, and not any
    actual definitions. When initializing functions/routines, length information
    is read from dd::Parameter and not this field.
  */
  sf->set_result_char_length(return_field->max_display_width_in_bytes());

  // Set result numeric precision.
  uint numeric_precision = 0;
  if (get_field_numeric_precision(return_field, &numeric_precision) == false)
    sf->set_result_numeric_precision(numeric_precision);

  // Set result numeric scale.
  uint scale = 0;
  if (get_field_numeric_scale(return_field, &scale) == false ||
      numeric_precision)
    sf->set_result_numeric_scale(scale);
  else
    DBUG_ASSERT(sf->is_result_numeric_scale_null());

  uint dt_precision = 0;
  if (get_field_datetime_precision(return_field, &dt_precision) == false)
    sf->set_result_datetime_precision(dt_precision);

  // Set result collation id.
  sf->set_result_collation_id(return_field->charset->number);
}

////////////////////////////////////////////////////////////////////////////////

/**
  Helper method for create_routine() to fill parameter information
  from the object of type Create_field.
  Method is called by the fill_routine_parameters_info().

  @param[in]  thd      Thread handle.
  @param[in]  field    Object of type Create_field.
  @param[out] param    Parameter object to be filled using the state of field
                       object.
*/

static void fill_parameter_info_from_field(THD *thd, Create_field *field,
                                           dd::Parameter *param) {
  DBUG_TRACE;

  // Set data type.
  param->set_data_type(get_new_field_type(field->sql_type));

  // We need a fake table and share to generate the default values.
  // We prepare these once, and reuse them for all fields.
  TABLE table;
  TABLE_SHARE share;
  table.s = &share;
  table.in_use = thd;
  table.s->db_low_byte_first = true;

  // Reset data type in utf8
  param->set_data_type_utf8(get_sql_type_by_create_field(&table, *field));

  // Set is_zerofill flag.
  param->set_zerofill(field->is_zerofill);

  // Set is_unsigned flag.
  param->set_unsigned(field->is_unsigned);

  // Set char length.
  param->set_char_length(field->max_display_width_in_bytes());

  // Set result numeric precision.
  uint numeric_precision = 0;
  if (get_field_numeric_precision(field, &numeric_precision) == false)
    param->set_numeric_precision(numeric_precision);

  // Set numeric scale.
  uint scale = 0;
  if (!get_field_numeric_scale(field, &scale))
    param->set_numeric_scale(scale);
  else
    DBUG_ASSERT(param->is_numeric_scale_null());

  uint dt_precision = 0;
  if (get_field_datetime_precision(field, &dt_precision) == false)
    param->set_datetime_precision(dt_precision);

  // Set geometry sub type
  if (field->sql_type == MYSQL_TYPE_GEOMETRY) {
    Properties *param_options = &param->options();
    param_options->set("geom_type", field->geom_type);
  }

  // Set elements of enum or set data type.
  if (field->interval) {
    DBUG_ASSERT(field->sql_type == MYSQL_TYPE_ENUM ||
                field->sql_type == MYSQL_TYPE_SET);

    const char **pos = field->interval->type_names;
    for (uint i = 0; *pos != nullptr; pos++, i++) {
      // Create enum/set object.
      Parameter_type_element *elem_obj = param->add_element();

      String_type interval_name(*pos, field->interval->type_lengths[i]);

      elem_obj->set_name(interval_name);
    }
  }

  // Set collation id.
  param->set_collation_id(field->charset->number);
}

////////////////////////////////////////////////////////////////////////////////

/**
  Helper method for create_routine() to fill parameters of routine to
  dd::Routine object from the sp_head.
  Method is called from the fill_dd_routine_info().

  @param[in]  thd        Thread handle.
  @param[in]  sp         Stored routine object.
  @param[out] routine    dd::Routine object prepared from sp_head.

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/

static bool fill_routine_parameters_info(THD *thd, sp_head *sp,
                                         Routine *routine) {
  DBUG_TRACE;

  /*
    The return type of the stored function is listed as first parameter from
    the Information_schema.parameters. Storing return type as first parameter
    for the stored functions.
  */
  if (sp->m_type == enum_sp_type::FUNCTION) {
    // Add parameter.
    dd::Parameter *param = routine->add_parameter();

    // Fill return type information.
    fill_parameter_info_from_field(thd, &sp->m_return_field_def, param);
  }

  // Fill parameter information of the stored routine.
  sp_pcontext *sp_root_parsing_ctx = sp->get_root_parsing_context();
  DBUG_ASSERT(sp_root_parsing_ctx != nullptr);
  for (uint i = 0; i < sp_root_parsing_ctx->context_var_count(); i++) {
    sp_variable *sp_var = sp_root_parsing_ctx->find_variable(i);
    Create_field *field_def = &sp_var->field_def;

    // Add parameter.
    dd::Parameter *param = routine->add_parameter();

    // Set parameter name.
    param->set_name(sp_var->name.str);

    // Set parameter mode.
    Parameter::enum_parameter_mode mode;
    switch (sp_var->mode) {
      case sp_variable::MODE_IN:
        mode = Parameter::PM_IN;
        break;
      case sp_variable::MODE_OUT:
        mode = Parameter::PM_OUT;
        break;
      case sp_variable::MODE_INOUT:
        mode = Parameter::PM_INOUT;
        break;
      default:
        DBUG_ASSERT(false); /* purecov: deadcode */
        return true;        /* purecov: deadcode */
    }
    param->set_mode(mode);

    // Fill return type information.
    fill_parameter_info_from_field(thd, field_def, param);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

/**
  Helper method for create_routine() to prepare dd::Routine object
  from the sp_head.

  @param[in]  thd        Thread handle.
  @param[in]  schema     Schema where the routine is to be created.
  @param[in]  sp         Stored routine object.
  @param[in]  definer    Definer of the routine.
  @param[out] routine    dd::Routine object to be prepared from the sp_head.

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/

static bool fill_dd_routine_info(THD *thd, const dd::Schema &schema,
                                 sp_head *sp, Routine *routine,
                                 const LEX_USER *definer) {
  DBUG_TRACE;

  // Set name.
  routine->set_name(sp->m_name.str);

  // Set definition.
  routine->set_definition(sp->m_body.str);

  // Set definition_utf8.
  routine->set_definition_utf8(sp->m_body_utf8.str);

  // Set parameter str for show routine operations.
  routine->set_parameter_str(sp->m_params.str);

  // Set is_deterministic.
  routine->set_deterministic(sp->m_chistics->detistic);

  // Set SQL data access.
  Routine::enum_sql_data_access daccess;
  enum_sp_data_access sp_daccess =
      (sp->m_chistics->daccess == SP_DEFAULT_ACCESS) ? SP_DEFAULT_ACCESS_MAPPING
                                                     : sp->m_chistics->daccess;
  switch (sp_daccess) {
    case SP_NO_SQL:
      daccess = Routine::SDA_NO_SQL;
      break;
    case SP_CONTAINS_SQL:
      daccess = Routine::SDA_CONTAINS_SQL;
      break;
    case SP_READS_SQL_DATA:
      daccess = Routine::SDA_READS_SQL_DATA;
      break;
    case SP_MODIFIES_SQL_DATA:
      daccess = Routine::SDA_MODIFIES_SQL_DATA;
      break;
    default:
      DBUG_ASSERT(false); /* purecov: deadcode */
      return true;        /* purecov: deadcode */
  }
  routine->set_sql_data_access(daccess);

  // Set security type.
  View::enum_security_type sec_type;
  enum_sp_suid_behaviour sp_suid = (sp->m_chistics->suid == SP_IS_DEFAULT_SUID)
                                       ? SP_DEFAULT_SUID_MAPPING
                                       : sp->m_chistics->suid;
  switch (sp_suid) {
    case SP_IS_SUID:
      sec_type = View::ST_DEFINER;
      break;
    case SP_IS_NOT_SUID:
      sec_type = View::ST_INVOKER;
      break;
    default:
      DBUG_ASSERT(false); /* purecov: deadcode */
      return true;        /* purecov: deadcode */
  }
  routine->set_security_type(sec_type);

  // Set definer.
  routine->set_definer(definer->user.str, definer->host.str);

  // Set sql_mode.
  routine->set_sql_mode(thd->variables.sql_mode);

  // Set client collation id.
  routine->set_client_collation_id(thd->charset()->number);

  // Set connection collation id.
  routine->set_connection_collation_id(
      thd->variables.collation_connection->number);

  // Set schema collation id.
  const CHARSET_INFO *db_cs = nullptr;
  if (get_default_db_collation(schema, &db_cs)) {
    DBUG_ASSERT(thd->is_error());
    return true;
  }
  if (db_cs == nullptr) db_cs = thd->collation();

  routine->set_schema_collation_id(db_cs->number);

  // Set comment.
  routine->set_comment(sp->m_chistics->comment.str ? sp->m_chistics->comment.str
                                                   : "");

  // Fill routine parameters
  return fill_routine_parameters_info(thd, sp, routine);
}

////////////////////////////////////////////////////////////////////////////////

bool create_routine(THD *thd, const Schema &schema, sp_head *sp,
                    const LEX_USER *definer) {
  DBUG_TRACE;

  bool error = false;
  // Create Function or Procedure object.
  if (sp->m_type == enum_sp_type::FUNCTION) {
    std::unique_ptr<Function> func(schema.create_function(thd));

    // Fill stored function return type.
    fill_dd_function_return_type(thd, sp, func.get());

    // Fill routine object.
    if (fill_dd_routine_info(thd, schema, sp, func.get(), definer)) return true;

    // Store routine metadata in DD table.
    enum_check_fields saved_check_for_truncated_fields =
        thd->check_for_truncated_fields;
    thd->check_for_truncated_fields = CHECK_FIELD_WARN;
    error = thd->dd_client()->store(func.get());
    thd->check_for_truncated_fields = saved_check_for_truncated_fields;
  } else {
    std::unique_ptr<Procedure> proc(schema.create_procedure(thd));

    // Fill routine object.
    if (fill_dd_routine_info(thd, schema, sp, proc.get(), definer)) return true;

    // Store routine metadata in DD table.
    enum_check_fields saved_check_for_truncated_fields =
        thd->check_for_truncated_fields;
    thd->check_for_truncated_fields = CHECK_FIELD_WARN;
    error = thd->dd_client()->store(proc.get());
    thd->check_for_truncated_fields = saved_check_for_truncated_fields;
  }

  return error;
}

////////////////////////////////////////////////////////////////////////////////

bool alter_routine(THD *thd, Routine *routine, st_sp_chistics *chistics) {
  DBUG_TRACE;

  // Set last altered time.
  routine->set_last_altered(
      dd::my_time_t_to_ull_datetime(thd->query_start_in_secs()));

  // Set security type.
  if (chistics->suid != SP_IS_DEFAULT_SUID) {
    View::enum_security_type sec_type;

    switch (chistics->suid) {
      case SP_IS_SUID:
        sec_type = View::ST_DEFINER;
        break;
      case SP_IS_NOT_SUID:
        sec_type = View::ST_INVOKER;
        break;
      default:
        DBUG_ASSERT(false); /* purecov: deadcode */
        return true;        /* purecov: deadcode */
    }

    routine->set_security_type(sec_type);
  }

  // Set sql data access.
  if (chistics->daccess != SP_DEFAULT_ACCESS) {
    Routine::enum_sql_data_access daccess;
    switch (chistics->daccess) {
      case SP_NO_SQL:
        daccess = Routine::SDA_NO_SQL;
        break;
      case SP_CONTAINS_SQL:
        daccess = Routine::SDA_CONTAINS_SQL;
        break;
      case SP_READS_SQL_DATA:
        daccess = Routine::SDA_READS_SQL_DATA;
        break;
      case SP_MODIFIES_SQL_DATA:
        daccess = Routine::SDA_MODIFIES_SQL_DATA;
        break;
      default:
        DBUG_ASSERT(false); /* purecov: deadcode */
        return true;        /* purecov: deadcode */
    }
    routine->set_sql_data_access(daccess);
  }

  // Set comment.
  if (chistics->comment.str) routine->set_comment(chistics->comment.str);

  // Update routine.
  return thd->dd_client()->update(routine);
}

////////////////////////////////////////////////////////////////////////////////
}  // namespace dd
