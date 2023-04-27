/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/dd_view.h"

#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <memory>
#include <string>

#include "lex_string.h"
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "my_time.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd.h"                       // dd::get_dictionary
#include "sql/dd/dd_table.h"                 // fill_dd_columns_from_create_*
#include "sql/dd/dictionary.h"               // dd::Dictionary
#include "sql/dd/impl/dictionary_impl.h"     // default_catalog_name
#include "sql/dd/impl/utils.h"               // dd::my_time_t_to_ull_datetime()
#include "sql/dd/properties.h"               // dd::Properties
#include "sql/dd/string_type.h"
#include "sql/dd/types/abstract_table.h"  // dd::enum_table_type
#include "sql/dd/types/schema.h"          // dd::Schema
#include "sql/dd/types/view.h"            // dd::View
#include "sql/dd/types/view_routine.h"    // dd::View_routine
#include "sql/dd/types/view_table.h"      // dd::View_table
#include "sql/dd_table_share.h"           // dd_get_mysql_charset
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/histograms/value_map.h"
#include "sql/item.h"
#include "sql/item_create.h"
#include "sql/item_func.h"  // Item_func
#include "sql/key.h"
#include "sql/log.h"
#include "sql/mem_root_array.h"
#include "sql/parse_file.h"  // PARSE_FILE_TIMESTAMPLENGTH
#include "sql/sp.h"          // Sroutine_hash_entry
#include "sql/sql_class.h"   // THD
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_servers.h"
#include "sql/sql_tmp_table.h"  // create_tmp_field
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_commit
#include "sql/tztime.h"       // Time_zone

namespace dd {

static ulonglong dd_get_old_view_check_type(dd::View::enum_check_option type) {
  switch (type) {
    case dd::View::CO_NONE:
      return VIEW_CHECK_NONE;

    case dd::View::CO_LOCAL:
      return VIEW_CHECK_LOCAL;

    case dd::View::CO_CASCADED:
      return VIEW_CHECK_CASCADED;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, "view check option.");
  DBUG_ASSERT(false);

  return VIEW_CHECK_NONE;
  /* purecov: end */
}

/** For enum in dd::View */
static dd::View::enum_check_option dd_get_new_view_check_type(ulonglong type) {
  switch (type) {
    case VIEW_CHECK_NONE:
      return dd::View::CO_NONE;

    case VIEW_CHECK_LOCAL:
      return dd::View::CO_LOCAL;

    case VIEW_CHECK_CASCADED:
      return dd::View::CO_CASCADED;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, "view check option.");
  DBUG_ASSERT(false);

  return dd::View::CO_NONE;
  /* purecov: end */
}

static enum enum_view_algorithm dd_get_old_view_algorithm_type(
    dd::View::enum_algorithm type) {
  switch (type) {
    case dd::View::VA_UNDEFINED:
      return VIEW_ALGORITHM_UNDEFINED;

    case dd::View::VA_TEMPORARY_TABLE:
      return VIEW_ALGORITHM_TEMPTABLE;

    case dd::View::VA_MERGE:
      return VIEW_ALGORITHM_MERGE;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, "view algorithm.");
  DBUG_ASSERT(false);

  return VIEW_ALGORITHM_UNDEFINED;
  /* purecov: end */
}

static dd::View::enum_algorithm dd_get_new_view_algorithm_type(
    enum enum_view_algorithm type) {
  switch (type) {
    case VIEW_ALGORITHM_UNDEFINED:
      return dd::View::VA_UNDEFINED;

    case VIEW_ALGORITHM_TEMPTABLE:
      return dd::View::VA_TEMPORARY_TABLE;

    case VIEW_ALGORITHM_MERGE:
      return dd::View::VA_MERGE;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, "view algorithm.");
  DBUG_ASSERT(false);

  return dd::View::VA_UNDEFINED;
  /* purecov: end */
}

static ulonglong dd_get_old_view_security_type(
    dd::View::enum_security_type type) {
  switch (type) {
    case dd::View::ST_DEFAULT:
      return VIEW_SUID_DEFAULT;

    case dd::View::ST_INVOKER:
      return VIEW_SUID_INVOKER;

    case dd::View::ST_DEFINER:
      return VIEW_SUID_DEFINER;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, "view security type.");
  DBUG_ASSERT(false);

  return VIEW_SUID_DEFAULT;
  /* purecov: end */
}

static dd::View::enum_security_type dd_get_new_view_security_type(
    ulonglong type) {
  switch (type) {
    case VIEW_SUID_DEFAULT:
      return dd::View::ST_DEFAULT;

    case VIEW_SUID_INVOKER:
      return dd::View::ST_INVOKER;

    case VIEW_SUID_DEFINER:
      return dd::View::ST_DEFINER;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, "view security type.");
  DBUG_ASSERT(false);

  return dd::View::ST_DEFAULT;
  /* purecov: end */
}

/**
  Method to fill view columns from the first SELECT_LEX of view query.

  @param  thd       Thread Handle.
  @param  view_obj  DD view object.
  @param  view      TABLE_LIST object of view.

  @retval false     On Success.
  @retval true      On failure.
*/

static bool fill_dd_view_columns(THD *thd, View *view_obj,
                                 const TABLE_LIST *view) {
  DBUG_TRACE;

  // Helper class which takes care restoration of THD::variables.sql_mode and
  // delete handler created for dummy table.
  class Context_handler {
   public:
    Context_handler(THD *thd, handler *file) : m_thd(thd), m_file(file) {
      m_sql_mode = m_thd->variables.sql_mode;
      m_thd->variables.sql_mode = 0;
    }
    ~Context_handler() {
      m_thd->variables.sql_mode = m_sql_mode;
      destroy(m_file);
    }

   private:
    // Thread Handle.
    THD *m_thd;

    // Handler object of dummy table.
    handler *m_file;

    // sql_mode.
    sql_mode_t m_sql_mode;
  };

  // Creating dummy TABLE and TABLE_SHARE objects to prepare Field objects from
  // the items of first SELECT_LEX of the view query. We prepare these once and
  // reuse them for all the fields.
  TABLE table;
  TABLE_SHARE share;
  init_tmp_table_share(thd, &share, "", 0, "", "", nullptr);
  table.s = &share;
  handler *file = get_new_handler(&share, false, thd->mem_root,
                                  ha_default_temp_handlerton(thd));
  if (file == nullptr) {
    my_error(ER_STORAGE_ENGINE_NOT_LOADED, MYF(0), view->db, view->table_name);
    return true;
  }

  Context_handler ctx_handler(thd, file);

  const dd::Properties &names_dict = view_obj->column_names();

  /*
    Iterate through all the items of first SELECT_LEX if view query is of
    single query block. Otherwise iterate through all the type holders items
    created for unioned column types of all the query blocks.
  */
  List_iterator_fast<Item> it(*(thd->lex->unit->get_unit_column_types()));
  List<Create_field> create_fields;
  Item *item;
  uint i = 0;
  while ((item = it++) != nullptr) {
    i++;
    bool is_sp_func_item = false;
    // Create temporary Field object from the item.
    Field *tmp_field;
    if (item->type() == Item::FUNC_ITEM) {
      if (item->result_type() != STRING_RESULT) {
        is_sp_func_item = ((static_cast<Item_func *>(item))->functype() ==
                           Item_func::FUNC_SP);
        /*
          INT Result values with MY_INT32_NUM_DECIMAL_DIGITS digits may or may
          not fit into Field_long so make them Field_longlong.
        */
        if (is_sp_func_item == false && item->result_type() == INT_RESULT &&
            item->max_char_length() >= (MY_INT32_NUM_DECIMAL_DIGITS - 1)) {
          tmp_field = new (thd->mem_root)
              Field_longlong(item->max_char_length(), item->maybe_null,
                             item->item_name.ptr(), item->unsigned_flag);
          if (tmp_field) tmp_field->init(&table);
        } else
          tmp_field = item->tmp_table_field(&table);
      } else {
        switch (item->data_type()) {
          case MYSQL_TYPE_TINY_BLOB:
          case MYSQL_TYPE_MEDIUM_BLOB:
          case MYSQL_TYPE_LONG_BLOB:
          case MYSQL_TYPE_BLOB:
            // Packlength is not set for blobs as per the length in
            // tmp_table_field_from_field_type, so creating the blob field by
            // passing set_packlenth value as "true" here.
            tmp_field = new (thd->mem_root) Field_blob(
                item->max_length, item->maybe_null, item->item_name.ptr(),
                item->collation.collation, true);

            if (tmp_field) tmp_field->init(&table);
            break;
          default:
            tmp_field = item->tmp_table_field_from_field_type(&table, false);
        }
      }
    } else {
      Field *from_field, *default_field;
      tmp_field = create_tmp_field(thd, &table, item, item->type(), nullptr,
                                   &from_field, &default_field, false, false,
                                   false, false, false);
    }
    if (!tmp_field) {
      my_error(ER_OUT_OF_RESOURCES, MYF(ME_FATALERROR));
      return true;
    }

    // We have to take into account both the real table's fields and
    // pseudo-fields used in trigger's body. These fields are used to copy
    // defaults values later inside constructor of the class Create_field.
    Field *orig_field = nullptr;
    if (item->type() == Item::FIELD_ITEM ||
        item->type() == Item::TRIGGER_FIELD_ITEM)
      orig_field = ((Item_field *)item)->field;

    // Create object of type Create_field from the tmp_field.
    Create_field *cr_field =
        new (thd->mem_root) Create_field(tmp_field, orig_field);
    if (cr_field == nullptr) {
      my_error(ER_OUT_OF_RESOURCES, MYF(ME_FATALERROR));
      return true;
    }

    if (!names_dict.empty())  // Explicit names were provided
    {
      std::string i_s = std::to_string(i);
      String_type value;
      char *name = nullptr;
      if (!names_dict.get(String_type(i_s.begin(), i_s.end()), &value)) {
        name = static_cast<char *>(
            strmake_root(thd->mem_root, value.c_str(), value.length()));
      }
      if (!name) return true; /* purecov: inspected */
      cr_field->field_name = name;
    } else if (thd->lex->unit->is_union()) {
      /*
        If view query has any duplicate column names then generated unique name
        is stored only with the first SELECT_LEX. So when Create_field instance
        is created with type holder item, store name from first SELECT_LEX.
      */
      cr_field->field_name =
          thd->lex->select_lex->item_list[i - 1]->item_name.ptr();
    }

    cr_field->after = nullptr;
    cr_field->offset = 0;
    cr_field->pack_length_override = 0;
    cr_field->is_nullable = !(tmp_field->flags & NOT_NULL_FLAG);
    cr_field->is_zerofill = (tmp_field->flags & ZEROFILL_FLAG);
    cr_field->is_unsigned = (tmp_field->flags & UNSIGNED_FLAG);

    create_fields.push_back(cr_field);
  }

  // Fill view columns information from the Create_field objects.
  return fill_dd_columns_from_create_fields(thd, view_obj, create_fields, file);
}

/**
  Method to fill base table and view names used by view query in DD
  View object.

  @param  view_obj       DD view object.
  @param  view           TABLE_LIST object of view.
  @param  query_tables   View query tables list.
*/

static void fill_dd_view_tables(View *view_obj, const TABLE_LIST *view,
                                const TABLE_LIST *query_tables) {
  DBUG_TRACE;

  for (const TABLE_LIST *table = query_tables; table != nullptr;
       table = table->next_global) {
    /*
      Skip if table is not directly referred by a view or if table is a
      data-dictionary or temporary table.
    */
    if ((table->referencing_view && table->referencing_view != view) ||
        get_dictionary()->is_dd_table_name(table->get_db_name(),
                                           table->get_table_name()) ||
        is_temporary_table(const_cast<TABLE_LIST *>(table)))
      continue;

    LEX_CSTRING db_name;
    LEX_CSTRING table_name;
    if (table->schema_table_name) {
      db_name = {table->db, table->db_length};
      table_name = {table->schema_table_name, strlen(table->schema_table_name)};
    } else if (table->is_view()) {
      db_name = table->view_db;
      table_name = table->view_name;
    } else {
      db_name = {table->db, table->db_length};
      table_name = {table->table_name, table->table_name_length};
    }

    // Avoid duplicate entries.
    {
      bool duplicate_vw = false;
      for (const View_table *vw : view_obj->tables()) {
        if (!strcmp(vw->table_schema().c_str(), db_name.str) &&
            !strcmp(vw->table_name().c_str(), table_name.str)) {
          duplicate_vw = true;
          break;
        }
      }
      if (duplicate_vw) continue;
    }

    View_table *view_table_obj = view_obj->add_table();

    // view table catalog
    view_table_obj->set_table_catalog(Dictionary_impl::default_catalog_name());

    // View table schema
    view_table_obj->set_table_schema(String_type(db_name.str, db_name.length));

    // View table name
    view_table_obj->set_table_name(
        String_type(table_name.str, table_name.length));
  }
}

/**
  Method to fill view routines from the set of routines used by view query.

  @param  view_obj      DD view object.
  @param  routines_ctx  Query_table_list object for the view which contains
                        set of routines used by view query.
*/

static void fill_dd_view_routines(View *view_obj,
                                  Query_tables_list *routines_ctx) {
  DBUG_TRACE;

  // View stored functions. We need only directly used routines.
  for (Sroutine_hash_entry *rt = routines_ctx->sroutines_list.first;
       rt != nullptr && rt != *routines_ctx->sroutines_list_own_last;
       rt = rt->next) {
    View_routine *view_sf_obj = view_obj->add_routine();

    /*
      We should get only stored functions here, as procedures are not directly
      used by views, and thus not stored as dependencies.
    */
    DBUG_ASSERT(rt->type() == Sroutine_hash_entry::FUNCTION);

    // view routine catalog
    view_sf_obj->set_routine_catalog(Dictionary_impl::default_catalog_name());

    // View routine schema
    view_sf_obj->set_routine_schema(String_type(rt->db(), rt->m_db_length));

    // View routine name
    view_sf_obj->set_routine_name(String_type(rt->name(), rt->name_length()));
  }
}

/**
  Method to fill view information in the View object.

  @param  thd          Thread handle.
  @param  view_obj     DD view object.
  @param  view         View description.

  @retval false        On success.
  @retval true         On failure.
*/

static bool fill_dd_view_definition(THD *thd, View *view_obj,
                                    TABLE_LIST *view) {
  // View name.
  view_obj->set_name(view->table_name);

  // Set definer.
  view_obj->set_definer(view->definer.user.str, view->definer.host.str);

  // View definition.
  view_obj->set_definition(
      String_type(view->select_stmt.str, view->select_stmt.length));

  view_obj->set_definition_utf8(
      String_type(view->view_body_utf8.str, view->view_body_utf8.length));

  // Set updatable.
  view_obj->set_updatable(view->updatable_view);

  // Set check option.
  view_obj->set_check_option(dd_get_new_view_check_type(view->with_check));

  // Set algorithm.
  view_obj->set_algorithm(dd_get_new_view_algorithm_type(
      (enum enum_view_algorithm)view->algorithm));

  // Set security type.
  view_obj->set_security_type(dd_get_new_view_security_type(view->view_suid));

  // Assign client collation ID. The create option specifies character
  // set name, and we store the default collation id for this character set
  // name, which implicitly identifies the character set.
  const CHARSET_INFO *collation = nullptr;
  if (resolve_charset(view->view_client_cs_name.str, system_charset_info,
                      &collation)) {
    // resolve_charset will not cause an error to be reported if the
    // character set was not found, so we must report error here.
    my_error(ER_UNKNOWN_CHARACTER_SET, MYF(0), view->view_client_cs_name.str);
    return true;
  }

  view_obj->set_client_collation_id(collation->number);

  // Assign connection collation ID.
  if (resolve_collation(view->view_connection_cl_name.str, system_charset_info,
                        &collation)) {
    // resolve_charset will not cause an error to be reported if the
    // collation was not found, so we must report error here.
    my_error(ER_UNKNOWN_COLLATION, MYF(0), view->view_connection_cl_name.str);
    return true;
  }

  view_obj->set_connection_collation_id(collation->number);

  if (view->derived_column_names()) {
    dd::Properties &names_dict = view_obj->column_names();
    uint i = 0;
    for (auto name : *view->derived_column_names()) {
      std::string i_s = std::to_string(++i);
      names_dict.set(String_type(i_s.begin(), i_s.end()),
                     String_type(name.str, name.length));
    }
  }

  time_t tm = my_time(0);
  get_date(view->timestamp.str,
           GETDATE_DATE_TIME | GETDATE_GMT | GETDATE_FIXEDLENGTH, tm);
  view->timestamp.length = PARSE_FILE_TIMESTAMPLENGTH;

  dd::Properties *view_options = &view_obj->options();
  view_options->set("timestamp",
                    String_type(view->timestamp.str, view->timestamp.length));
  view_options->set("view_valid", true);

  /*
    Fill view columns information in View object.

    During DD upgrade, view metadata is stored in 2 phases. In first phase,
    view metadata is stored without column information. In second phase view
    metadata stored with column information. Fill view columns only when view
    metadata is stored with column information.
  */
  if ((thd->lex->select_lex->item_list.elements > 0) &&
      fill_dd_view_columns(thd, view_obj, view))
    return true;

  // Fill view tables information in View object.
  fill_dd_view_tables(view_obj, view, thd->lex->query_tables);

  /*
    Fill view routines information in View object. It is important that
    THD::lex points to the view's LEX at this point, so information about
    directly used routines in it is correct.
  */
  fill_dd_view_routines(view_obj, thd->lex);

  return false;
}

bool update_view(THD *thd, dd::View *new_view, TABLE_LIST *view) {
  // Clear the columns, tables and routines since it will be added later.
  new_view->remove_children();

  // Get statement start time.
  // Set last altered time.
  new_view->set_last_altered(
      dd::my_time_t_to_ull_datetime(thd->query_start_in_secs()));

  if (fill_dd_view_definition(thd, new_view, view)) return true;

  // Update DD tables.
  return thd->dd_client()->update(new_view);
}

bool create_view(THD *thd, const dd::Schema &schema, TABLE_LIST *view) {
  // Create dd::View object.
  bool hidden_system_view = false;
  std::unique_ptr<dd::View> view_obj;
  if (dd::get_dictionary()->is_system_view_name(view->db, view->table_name,
                                                &hidden_system_view)) {
    view_obj.reset(schema.create_system_view(thd));

    // Mark the internal system views as hidden from users.
    if (hidden_system_view)
      view_obj->set_hidden(dd::Abstract_table::HT_HIDDEN_SYSTEM);
  } else
    view_obj.reset(schema.create_view(thd));

  if (fill_dd_view_definition(thd, view_obj.get(), view)) return true;

  // Store info in DD views table.
  return thd->dd_client()->store(view_obj.get());
}

bool read_view(TABLE_LIST *view, const dd::View &view_obj, MEM_ROOT *mem_root) {
  // Fill TABLE_LIST 'view' with view details.
  String_type definer_user = view_obj.definer_user();
  view->definer.user.length = definer_user.length();
  view->definer.user.str = (char *)strmake_root(mem_root, definer_user.c_str(),
                                                definer_user.length());

  String_type definer_host = view_obj.definer_host();
  view->definer.host.length = definer_host.length();
  view->definer.host.str = (char *)strmake_root(mem_root, definer_host.c_str(),
                                                definer_host.length());

  // View definition body.
  String_type vd_utf8 = view_obj.definition_utf8();
  view->view_body_utf8.length = vd_utf8.length();
  view->view_body_utf8.str =
      (char *)strmake_root(mem_root, vd_utf8.c_str(), vd_utf8.length());

  // Get updatable.
  view->updatable_view = view_obj.is_updatable();

  // Get check option.
  view->with_check = dd_get_old_view_check_type(view_obj.check_option());

  // Get algorithm.
  view->algorithm = dd_get_old_view_algorithm_type(view_obj.algorithm());

  // Get security type.
  view->view_suid = dd_get_old_view_security_type(view_obj.security_type());

  // Mark true, if we are reading a system view.
  view->is_system_view = (view_obj.type() == dd::enum_table_type::SYSTEM_VIEW);

  // Get definition.
  String_type view_definition = view_obj.definition();
  view->select_stmt.length = view_definition.length();
  view->select_stmt.str = (char *)strmake_root(
      mem_root, view_definition.c_str(), view_definition.length());

  // Get view_client_cs_name. Note that this is the character set name.
  CHARSET_INFO *collation =
      dd_get_mysql_charset(view_obj.client_collation_id());
  DBUG_ASSERT(collation);
  view->view_client_cs_name.length = strlen(collation->csname);
  view->view_client_cs_name.str = strdup_root(mem_root, collation->csname);

  // Get view_connection_cl_name. Note that this is the collation name.
  collation = dd_get_mysql_charset(view_obj.connection_collation_id());
  DBUG_ASSERT(collation);
  view->view_connection_cl_name.length = strlen(collation->name);
  view->view_connection_cl_name.str = strdup_root(mem_root, collation->name);

  if (!(view->definer.user.str && view->definer.host.str &&  // OOM
        view->view_body_utf8.str && view->select_stmt.str &&
        view->view_client_cs_name.str && view->view_connection_cl_name.str))
    return true; /* purecov: inspected */

  const dd::Properties &names_dict = view_obj.column_names();
  if (!names_dict.empty())  // Explicit names were provided
  {
    auto *names_array = static_cast<Create_col_name_list *>(
        mem_root->Alloc(sizeof(Create_col_name_list)));
    if (!names_array) return true; /* purecov: inspected */
    names_array->init(mem_root);
    uint i = 0;
    while (true) {
      std::string i_s = std::to_string(++i);
      String_type key(i_s.begin(), i_s.end());
      if (!names_dict.exists(key)) break;
      String_type value;
      names_dict.get(key, &value);
      char *name = static_cast<char *>(
          strmake_root(mem_root, value.c_str(), value.length()));
      if (!name || (names_array->push_back({name, value.length()})))
        return true; /* purecov: inspected */
    }
    view->set_derived_column_names(names_array);
  }
  return false;
}

bool update_view_status(THD *thd, const char *schema_name,
                        const char *view_name, bool status,
                        bool commit_dd_changes) {
  dd::cache::Dictionary_client *client = thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser(client);
  dd::View *new_view = nullptr;
  if (client->acquire_for_modification(schema_name, view_name, &new_view))
    return true;
  if (new_view == nullptr) return false;

  // Update view error status.
  dd::Properties *view_options = &new_view->options();
  view_options->set("view_valid", status);

  Implicit_substatement_state_guard substatement_guard(thd);

  // Update DD tables.
  if (client->update(new_view)) {
    if (commit_dd_changes) {
      trans_rollback_stmt(thd);
      trans_rollback(thd);
    }
    return true;
  }

  return commit_dd_changes && (trans_commit_stmt(thd) || trans_commit(thd));
}

}  // namespace dd
