/*
   Copyright (c) 2005, 2015, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/*
  This engine need server classes (like THD etc.) which only is defined if
  MYSQL_SERVER define is set!
*/
#include "my_io.h"
#include "sql/handler.h"
#include "sql/sql_partition.h"
#define MYSQL_SERVER 1
#define LOG_SUBSYSTEM_TAG "partition_base"

#include "mysql/psi/mysql_file.h"
#include "partition_base.h"
#include "pfs_file_provider.h"
#include "sql/partition_info.h"  // partition_info
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"  // append_file_to_dir

#include "myisam.h"                                  // TT_FOR_UPGRADE
#include "mysql/components/services/log_builtins.h"  // print error messages
#include "sql/key.h"         // key_rec_cmp, field_unpack
#include "sql/mysqld.h"      // opt_parthandler_allow_drop_partition
#include "sql/sql_admin.h"   // SQL_ADMIN_MSG_TEXT_SIZE
#include "sql/sql_plugin.h"  // plugin_unlock_list
#include "sql/sql_show.h"    // append_identifier
#include "sql/sql_table.h"   // tablename_to_filename
#include "sql/thd_raii.h"
#include "varlen_sort.h"

#include "sql/dd/dd.h"
#include "sql/dd/dictionary.h"
#include "sql/dd/properties.h"
#include "sql/dd/types/partition.h"
#include "sql/dd/types/table.h"

#include <algorithm>
#include <string>
#include <vector>

#include "sql/debug_sync.h"
#ifndef DBUG_OFF
#include "sql/sql_test.h"  // print_where
#endif

#include "mysql/psi/mysql_file.h"
#include "pfs_file_provider.h"

#include <boost/scope_exit.hpp>

using std::max;
using std::min;

namespace native_part {

/* First 4 bytes in the .par file is the number of 32-bit words in the file */
#define PAR_WORD_SIZE 4
/* offset to the .par file checksum */
#define PAR_CHECKSUM_OFFSET 4
/* offset to the total number of partitions */
#define PAR_NUM_PARTS_OFFSET 8
/* offset to the engines array */
#define PAR_ENGINES_OFFSET 12
#define PARTITION_ENABLED_TABLE_FLAGS (HA_FILE_BASED | HA_CAN_REPAIR)
#define PARTITION_DISABLED_TABLE_FLAGS                    \
  (HA_CAN_GEOMETRY | HA_CAN_FULLTEXT | HA_DUPLICATE_POS | \
   HA_READ_BEFORE_WRITE_REMOVAL)
/** operation names for the enum_part_operation. */
static const char *opt_op_name[] = {
    "optimize",           "analyze",     "check", "repair",
    "assign_to_keycache", "preload_keys"};

/****************************************************************************
                MODULE create/delete handler object
****************************************************************************/

static PSI_memory_key key_memory_Partition_base_part_ids;
PSI_file_key key_file_Partition_base_par;

static void create_partition_name(char *out, const char *in1, const char *in2,
                                  bool translate) {
  char transl_part_name[FN_REFLEN];
  const char *transl_part;

  if (translate) {
    tablename_to_filename(in2, transl_part_name, FN_REFLEN);
    transl_part = transl_part_name;
  } else
    transl_part = in2;
  strxmov(out, in1, "#P#", transl_part, NullS);
}

static void create_subpartition_name(char *out, const char *in1,
                                     const char *in2, const char *in3) {
  char transl_part_name[FN_REFLEN], transl_subpart_name[FN_REFLEN];

  tablename_to_filename(in2, transl_part_name, FN_REFLEN);
  tablename_to_filename(in3, transl_subpart_name, FN_REFLEN);
  strxmov(out, in1, "#P#", transl_part_name, "#SP#", transl_subpart_name,
          NullS);
}

void part_name(char *out_buf, const char *path, const char *parent_elem_name,
               const char *elem_name) {
  static const char *sp_prefix = "#SP#";
  static const size_t sp_prefix_length = 4;

  char part_name[FN_REFLEN];
  size_t part_name_length = tablename_to_filename(
      parent_elem_name ? parent_elem_name : elem_name, part_name, FN_REFLEN);
  if (parent_elem_name) {
    DBUG_ASSERT(part_name_length + sp_prefix_length < sizeof(part_name));
    strncat(part_name, sp_prefix, sizeof(part_name) - part_name_length - 1);
    part_name_length += sp_prefix_length;
    char subpart_name[FN_REFLEN];
    size_t subpart_name_length =
        tablename_to_filename(elem_name, subpart_name, FN_REFLEN);
    DBUG_ASSERT(part_name_length + subpart_name_length < sizeof(part_name));
    strncat(part_name, subpart_name, sizeof(part_name) - part_name_length - 1);
    part_name_length += subpart_name_length;
  }

  create_partition_name(out_buf, path, part_name, false);
}

Parts_share_refs::Parts_share_refs() : num_parts(0), ha_shares(nullptr) {}

Parts_share_refs::~Parts_share_refs() {
  if (ha_shares) {
    for (uint i = 0; i < num_parts; i++)
      if (ha_shares[i]) delete ha_shares[i];
    delete[] ha_shares;
  }
}

bool Parts_share_refs::init(uint arg_num_parts) {
  DBUG_ASSERT(!num_parts && !ha_shares);
  num_parts = arg_num_parts;
  /* Allocate an array of Handler_share pointers */
  ha_shares = new Handler_share *[num_parts];
  if (!ha_shares) {
    num_parts = 0;
    return true;
  }
  memset(ha_shares, 0, sizeof(Handler_share *) * num_parts);
  return false;
}

Partition_base_share::Partition_base_share()
    : Partition_share(), partitions_share_refs(nullptr) {}

Partition_base_share::~Partition_base_share() {
  if (partitions_share_refs) delete partitions_share_refs;
}

/**
  Initialize handler before start of index scan.

  index_init is always called before starting index scans (except when
  starting through index_read_idx and using read_range variants).

  @param inx     Index number.
  @param sorted  Is rows to be returned in sorted order.

  @return Operation status
    @retval    0  Success
    @retval != 0  Error code

  @note Ported from 5.7 Partition_helper::ph_index_init().
*/

int Partition_base::index_init(uint inx, bool sorted) {
  int error;
  uint part_id = m_part_info->get_first_used_partition();
  DBUG_ENTER("Partition_helper::index_init");
  set_active_index(inx);

  if (part_id == MY_BIT_NONE) {
    DBUG_RETURN(0);
  }

  if ((error = ph_index_init_setup(inx, sorted))) {
    DBUG_RETURN(error);
  }
  if ((error = init_record_priority_queue())) {
    destroy_record_priority_queue();
    DBUG_RETURN(error);
  }

  for (/* part_id already set. */; part_id < MY_BIT_NONE;
       part_id = m_part_info->get_next_used_partition(part_id)) {
    if ((error = index_init_in_part(part_id, inx, sorted))) goto err;

    DBUG_EXECUTE_IF("partition_fail_index_init", {
      part_id++;
      error = HA_ERR_NO_PARTITION_FOUND;
      goto err;
    });
  }
err:
  if (error) {
    /* End the previously initialized indexes. */
    uint j;
    for (j = m_part_info->get_first_used_partition(); j < part_id;
         j = m_part_info->get_next_used_partition(j)) {
      (void)index_end_in_part(j);
    }
    destroy_record_priority_queue();
  }
  DBUG_RETURN(error);
}

/**
  End of index scan.

  index_end is called at the end of an index scan to clean up any
  things needed to clean up.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code

  @note Ported from 5.7 Partition_helper::ph_index_end().
*/

int Partition_base::index_end() {
  int error = 0;
  uint i;
  DBUG_ENTER("Partition_helper::ph_index_end");

  m_part_spec.start_part = NO_CURRENT_PART_ID;
  m_ref_usage = REF_NOT_USED;
  for (i = m_part_info->get_first_used_partition(); i < MY_BIT_NONE;
       i = m_part_info->get_next_used_partition(i)) {
    int tmp;
    if ((tmp = index_end_in_part(i))) error = tmp;
  }
  destroy_record_priority_queue();
  set_active_index(MAX_KEY);
  DBUG_RETURN(error);
}

/**
  Read row using position.

  This is like rnd_next, but you are given a position to use to determine
  the row. The position will be pointing to data of length handler::ref_length
  that handler::ref was set by position(record). Tables clustered on primary
  key usually use the full primary key as reference (like InnoDB). Heap based
  tables usually returns offset in heap file (like MyISAM).

  @param[out] buf  buffer that should be filled with record in MySQL format.
  @param[in]  pos  position given as handler::ref when position() was called.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code

  @note Ported from 5.7 Partition_helper::ph_rnd_pos()
*/

int Partition_base::rnd_pos(uchar *buf, uchar *pos) {
  uint part_id;
  DBUG_ENTER("Partition_helper::ph_rnd_pos");

  part_id = uint2korr(pos);
  DBUG_ASSERT(part_id < m_tot_parts);
  DBUG_ASSERT(m_part_info->is_partition_used(part_id));
  m_last_part = part_id;
  DBUG_RETURN(rnd_pos_in_part(part_id, buf, (pos + PARTITION_BYTES_IN_POS)));
}

/**
  Initialize and allocate space for partitions shares.

  @param num_parts  Number of partitions to allocate storage for.

  @return Operation status.
    @retval true  Failure (out of memory).
    @retval false Success.
*/

bool Partition_base_share::init(uint num_parts) {
  DBUG_ENTER("Partition_base_share::init");
  partitions_share_refs = new Parts_share_refs;
  if (!partitions_share_refs) DBUG_RETURN(true);
  if (partitions_share_refs->init(num_parts)) {
    delete partitions_share_refs;
    DBUG_RETURN(true);
  }
  DBUG_RETURN(false);
}

const uint32 Partition_base::NO_CURRENT_PART_ID = NOT_A_PARTITION_ID;

/*
  Constructor method

  SYNOPSIS
    Partition_base()
    table                       Table object

  RETURN VALUE
    NONE
*/

Partition_base::Partition_base(handlerton *hton, TABLE_SHARE *share)
    : handler(hton, share),
      Partition_helper(this),
      m_clone_base(nullptr),
      m_clone_mem_root(nullptr) {
  DBUG_ENTER("Partition_base::Partition_base(table)");
  init_handler_variables();
  DBUG_VOID_RETURN;
}

Partition_base::Partition_base(handlerton *hton, TABLE_SHARE *share,
                               Partition_base *clone_base,
                               MEM_ROOT *clone_mem_root)
    : handler(hton, share),
      Partition_helper(this),
      m_clone_base(clone_base),
      m_clone_mem_root(clone_mem_root) {
  DBUG_ENTER("Partition_base::Partition_base(table, ha_share)");
  init_handler_variables();
  set_ha_share_ref(&share->ha_share);
  DBUG_VOID_RETURN;
}

/*
  Initialize handler object

  SYNOPSIS
    init_handler_variables()

  RETURN VALUE
    NONE
*/

void Partition_base::init_handler_variables() {
  active_index = MAX_KEY;
  m_mode = 0;
  m_file = nullptr;
  m_file_tot_parts = 0;
  m_tot_parts = 0;
  m_pkey_is_clustered = false;
  m_handler_status = handler_not_initialized;
  m_part_func_monotonicity_info = NON_MONOTONIC;
  /*
    this allows blackhole to work properly
  */
  part_share = nullptr;
  m_new_partitions_share_refs.empty();
  m_part_ids_sorted_by_num_of_records = nullptr;
  m_new_file = nullptr;
  m_num_new_partitions = 0;
  m_indexes_are_disabled = false;
}

const char *Partition_base::table_type() const {
  // we can do this since we only support a single engine type
  return m_file[0]->table_type();
}

/*
  Destructor method

  SYNOPSIS
    ~Partition_base()

  RETURN VALUE
    NONE
*/

Partition_base::~Partition_base() {
  DBUG_ENTER("Partition_base::~Partition_base()");
  if (m_new_partitions_share_refs.elements)
    m_new_partitions_share_refs.delete_elements();
  if (m_file != nullptr) {
    for (uint i = 0; i < m_tot_parts; i++) {
      destroy(m_file[i]);
      m_file[i] = nullptr;
    }
  }
  if (m_new_file != nullptr) {
    for (uint i = 0; i < m_num_new_partitions; i++) {
      destroy(m_new_file[i]);
      m_new_file[i] = nullptr;
    }
  }
  my_free(m_part_ids_sorted_by_num_of_records);

  DBUG_VOID_RETURN;
}

bool Partition_base::init_with_fields() {
  /* Partition info has not yet been initialized, just return true */
  if (m_handler_status != handler_initialized) return false;
  /* Pass the call to each partition */
  for (uint i = 0; i < m_tot_parts; i++) {
    if (m_file[i]->init_with_fields()) return true;
  }
  /* Re-read table flags in case init_with_fields caused it to change */
  cached_table_flags =
      (m_file[0]->ha_table_flags() & ~(PARTITION_DISABLED_TABLE_FLAGS)) |
      PARTITION_ENABLED_TABLE_FLAGS;
  return false;
}

template <typename Fn>
bool Partition_base::foreach_partition(const Fn &fn) {
  DBUG_ASSERT(m_part_info);
  List_iterator_fast<partition_element> part_it(m_part_info->partitions);
  for (uint i = 0; i < m_part_info->num_parts; ++i) {
    partition_element *part_elem = part_it++;
    if (m_is_sub_partitioned) {
      List_iterator_fast<partition_element> sub_it(part_elem->subpartitions);
      for (uint j = 0; j < m_part_info->num_subparts; ++j) {
        partition_element *sub_part_elem = sub_it++;
        if (!fn(part_elem, sub_part_elem)) return false;
      }
    } else {
      if (!fn(nullptr, part_elem)) return false;
    }
  }
  return true;
}

/*
  Initialize partition handler object

  SYNOPSIS
    initialize_partition()
    mem_root                    Allocate memory through this

  RETURN VALUE
    1                         Error
    0                         Success

  DESCRIPTION

  The partition handler is only a layer on top of other engines. Thus it
  can't really perform anything without the underlying handlers. Thus we
  add this method as part of the allocation of a handler object.

  1) Allocation of underlying handlers
     If we have access to the partition info we will allocate one handler
     instance for each partition.
  2) Allocation without partition info
     The cases where we don't have access to this information is when called
     in preparation for delete_table and rename_table and in that case we
     only need to set HA_FILE_BASED.
  3) Table flags initialisation
     We need also to set table flags for the partition handler. This is not
     static since it depends on what storage engines are used as underlying
     handlers.
     The table flags is set in this routine to simulate the behaviour of a
     normal storage engine
     The flag HA_FILE_BASED will be set independent of the underlying handlers
  4) Index flags initialisation
     When knowledge exists on the indexes it is also possible to initialize the
     index flags. Again the index flags must be initialized by using the under-
     lying handlers since this is storage engine dependent.
     The flag HA_READ_ORDER will be reset for the time being to indicate no
     ordered output is available from partition handler indexes. Later a merge
     sort will be performed using the underlying handlers.
  5) primary_key_is_clustered, has_transactions and low_byte_first is
     calculated here.

*/

bool Partition_base::initialize_partition(MEM_ROOT *mem_root) {
  handler **file_array, *file;
  ulonglong check_table_flags;
  DBUG_ENTER("Partition_base::initialize_partition");

  if (m_part_info) {
    DBUG_ASSERT(m_tot_parts > 0);
    if (new_handlers_from_part_info(mem_root)) DBUG_RETURN(true);
  } else if (!table_share || !table_share->normalized_path.str) {
    /*
      Called with dummy table share (delete, rename and alter table).
      Don't need to set-up anything.
    */
    DBUG_RETURN(false);
  }
  /*
    We create all underlying table handlers here. We do it in this special
    method to be able to report allocation errors.

    Set up low_byte_first, primary_key_is_clustered and
    has_transactions since they are called often in all kinds of places,
    other parameters are calculated on demand.
    Verify that all partitions have the same table_flags.
  */
  check_table_flags = m_file[0]->ha_table_flags();
  m_pkey_is_clustered = true;
  file_array = m_file;
  do {
    file = *file_array;
    if (!file->primary_key_is_clustered()) m_pkey_is_clustered = false;
    if (check_table_flags != file->ha_table_flags()) {
      my_error(ER_MIX_HANDLER_ERROR, MYF(0));
      DBUG_RETURN(true);
    }
  } while (*(++file_array));
  m_handler_status = handler_initialized;
  DBUG_RETURN(false);
}

/****************************************************************************
                MODULE meta data changes
****************************************************************************/
/*
  Delete a table

  SYNOPSIS
    delete_table()
    name                    Full path of table name

  RETURN VALUE
    >0                        Error
    0                         Success

  DESCRIPTION
    Used to delete a table. By the time delete_table() has been called all
    opened references to this table will have been closed (and your globally
    shared references released. The variable name will just be the name of
    the table. You will need to remove any files you have created at this
    point.

    If you do not implement this, the default delete_table() is called from
    handler.cc and it will delete all files with the file extentions returned
    by bas_ext().

    Called from handler.cc by delete_table and  ha_create_table(). Only used
    during create if the table_flag HA_DROP_BEFORE_CREATE was specified for
    the storage engine.
*/

int Partition_base::delete_table(const char *name, const dd::Table *table_def) {
  DBUG_ENTER("Partition_base::delete_table");

  DBUG_RETURN(del_ren_table(name, nullptr, table_def, nullptr));
}

/*
  Rename a table

  SYNOPSIS
    rename_table()
    from                      Full path of old table name
    to                        Full path of new table name

  RETURN VALUE
    >0                        Error
    0                         Success

  DESCRIPTION
    Renames a table from one name to another from alter table call.

    If you do not implement this, the default rename_table() is called from
    handler.cc and it will rename all files with the file extentions returned
    by bas_ext().

    Called from sql_table.cc by mysql_rename_table().
*/

int Partition_base::rename_table(const char *from, const char *to,
                                 const dd::Table *table_def_from,
                                 dd::Table *table_def_to) {
  DBUG_ENTER("Partition_base::rename_table");

  DBUG_RETURN(del_ren_table(from, to, table_def_from, table_def_to));
}

/*
  Create a partitioned table

  SYNOPSIS
    create()
    name                              Full path of table name
    table_arg                         Table object
    create_info                       Create info generated for CREATE TABLE

  RETURN VALUE
    >0                        Error
    0                         Success

  DESCRIPTION
    create() is called to create a table. The variable name will have the name
    of the table. When create() is called you do not need to worry about
    opening the table.

    Called from handler.cc by ha_create_table().
*/

int Partition_base::create(const char *name, TABLE *table_arg,
                           HA_CREATE_INFO *create_info, dd::Table *table_def) {
  int error;
  char name_lc_buff[FN_REFLEN];
  std::vector<std::string> part_name_collection;
  const char *path;
  List_iterator_fast<partition_element> part_it(m_part_info->partitions);
  partition_element table_level_options;
  handler **file, **abort_file;
  DBUG_ENTER("Partition_base::create");

  DBUG_ASSERT(*fn_rext(const_cast<char *>(name)) == '\0');

  /* Not allowed to create temporary partitioned tables */
  if (create_info && create_info->options & HA_LEX_CREATE_TMP_TABLE) {
    my_error(ER_PARTITION_NO_TEMPORARY, MYF(0));
    DBUG_RETURN(true);
  }

  /*
    To initialize partitioning and part_share we need to have m_part_info
    filled in.
  */
  if (initialize_partition(&table_share->mem_root) || init_part_share())
    DBUG_RETURN(true);
  file = m_file;
  /*
    Since Partition_base has HA_FILE_BASED, it must alter underlying table names
    if they do not have HA_FILE_BASED and lower_case_table_names == 2.
    See Bug#37402, for Mac OS X.
    The appended #P#<partname>[#SP#<subpartname>] will remain in current case.
    Using the first partitions handler, since mixing handlers is not allowed.
  */
  path = get_canonical_filename(*file, name, name_lc_buff);
  table_level_options.set_from_info(create_info);

  if (foreach_partition([&](partition_element *parent_elem,
                            partition_element *part_elem) -> bool {
        char name_buff[FN_REFLEN];
        part_name(name_buff, path,
                  parent_elem ? parent_elem->partition_name : nullptr,
                  part_elem->partition_name);
        const char *old_data_file_name = nullptr;
        if (part_elem->data_file_name) {
          old_data_file_name = create_info->data_file_name;
          create_info->data_file_name = part_elem->data_file_name;
        }
        const char *old_index_file_name = nullptr;
        if (part_elem->index_file_name) {
          old_index_file_name = create_info->data_file_name;
          create_info->index_file_name = part_elem->index_file_name;
        }
        BOOST_SCOPE_EXIT_ALL(&) {
          if (part_elem->data_file_name)
            create_info->data_file_name = old_data_file_name;
          if (part_elem->index_file_name)
            create_info->index_file_name = old_index_file_name;
        };
        if ((error = (*file)->ha_create(name_buff, table_arg, create_info,
                                        table_def)))
          return false;

        table_level_options.put_to_info(create_info);
        ++file;
        part_name_collection.push_back(name_buff);
        return true;
      }))
    DBUG_RETURN(0);

  auto part_name_i = part_name_collection.begin();
  for (abort_file = file, file = m_file; file < abort_file;
       ++file, ++part_name_i) {
    (void)(*file)->ha_delete_table(part_name_i->c_str(), table_def);
  }
  handler::delete_table(name, table_def);
  DBUG_RETURN(error);
}

/*
  Optimize table

  SYNOPSIS
    optimize()
    thd               Thread object
    check_opt         Check/analyze/repair/optimize options

  RETURN VALUES
    >0                Error
    0                 Success
*/

int Partition_base::optimize(THD *thd, HA_CHECK_OPT *check_opt) {
  DBUG_ENTER("Partition_base::optimize");

  DBUG_RETURN(handle_opt_partitions(thd, check_opt, OPTIMIZE_PARTS));
}

/*
  Analyze table

  SYNOPSIS
    analyze()
    thd               Thread object
    check_opt         Check/analyze/repair/optimize options

  RETURN VALUES
    >0                Error
    0                 Success
*/

int Partition_base::analyze(THD *thd, HA_CHECK_OPT *check_opt) {
  DBUG_ENTER("Partition_base::analyze");

  int result = handle_opt_partitions(thd, check_opt, ANALYZE_PARTS);

  DBUG_RETURN(result);
}

/*
  Check table

  SYNOPSIS
    check()
    thd               Thread object
    check_opt         Check/analyze/repair/optimize options

  RETURN VALUES
    >0                Error
    0                 Success
*/

int Partition_base::check(THD *thd, HA_CHECK_OPT *check_opt) {
  DBUG_ENTER("Partition_base::check");

  DBUG_RETURN(handle_opt_partitions(thd, check_opt, CHECK_PARTS));
}

/*
  Repair table

  SYNOPSIS
    repair()
    thd               Thread object
    check_opt         Check/analyze/repair/optimize options

  RETURN VALUES
    >0                Error
    0                 Success
*/

int Partition_base::repair(THD *thd, HA_CHECK_OPT *check_opt) {
  DBUG_ENTER("Partition_base::repair");

  DBUG_RETURN(handle_opt_partitions(thd, check_opt, REPAIR_PARTS));
}

/**
  Get checksum for table.

  @return Checksum or 0 if not supported, which also may be a correct checksum!.

  @note Ported from 5.7 Partition_helper::ph_checksum().
*/
ha_checksum Partition_base::checksum() const {
  ha_checksum sum = 0;
  if (ha_table_flags() & HA_HAS_CHECKSUM) {
    for (uint i = 0; i < m_tot_parts; i++) {
      sum += checksum_in_part(i);
    }
  }
  return sum;
}

/**
  Assign to keycache

  @param thd          Thread object
  @param check_opt    Check/analyze/repair/optimize options

  @return
    @retval >0        Error
    @retval 0         Success
*/

int Partition_base::assign_to_keycache(THD *thd, HA_CHECK_OPT *check_opt) {
  DBUG_ENTER("Partition_base::assign_to_keycache");

  DBUG_RETURN(handle_opt_partitions(thd, check_opt, ASSIGN_KEYCACHE_PARTS));
}

/**
  Preload to keycache

  @param thd          Thread object
  @param check_opt    Check/analyze/repair/optimize options

  @return
    @retval >0        Error
    @retval 0         Success
*/

int Partition_base::preload_keys(THD *thd, HA_CHECK_OPT *check_opt) {
  DBUG_ENTER("Partition_base::preload_keys");

  DBUG_RETURN(handle_opt_partitions(thd, check_opt, PRELOAD_KEYS_PARTS));
}

/*
  Handle optimize/analyze/check/repair of one partition

  SYNOPSIS
    handle_opt_part()
    thd                      Thread object
    check_opt                Options
    file                     Handler object of partition
    flag                     Optimize/Analyze/Check/Repair flag

  RETURN VALUE
    >0                        Failure
    0                         Success
*/

int Partition_base::handle_opt_part(THD *thd, HA_CHECK_OPT *check_opt,
                                    uint part_id,
                                    enum_part_operation operation) {
  int error;
  handler *file = m_file[part_id];
  DBUG_ENTER("handle_opt_part");
  DBUG_PRINT("enter", ("operation = %u", operation));

  if (operation == OPTIMIZE_PARTS)
    error = file->ha_optimize(thd, check_opt);
  else if (operation == ANALYZE_PARTS)
    error = file->ha_analyze(thd, check_opt);
  else if (operation == CHECK_PARTS) {
    error = file->ha_check(thd, check_opt);
    if (!error || error == HA_ADMIN_ALREADY_DONE ||
        error == HA_ADMIN_NOT_IMPLEMENTED) {
      if (check_opt->flags & (T_MEDIUM | T_EXTEND))
        error = Partition_helper::check_misplaced_rows(part_id, false);
    }
  } else if (operation == REPAIR_PARTS) {
    error = file->ha_repair(thd, check_opt);
    if (!error || error == HA_ADMIN_ALREADY_DONE ||
        error == HA_ADMIN_NOT_IMPLEMENTED) {
      if (check_opt->flags & (T_MEDIUM | T_EXTEND))
        error = Partition_helper::check_misplaced_rows(part_id, true);
    }
  } else if (operation == ASSIGN_KEYCACHE_PARTS)
    error = file->assign_to_keycache(thd, check_opt);
  else if (operation == PRELOAD_KEYS_PARTS)
    error = file->preload_keys(thd, check_opt);
  else {
    DBUG_ASSERT(false);
    error = 1;
  }
  if (error == HA_ADMIN_ALREADY_DONE) error = 0;
  DBUG_RETURN(error);
}

/*
  Handle optimize/analyze/check/repair of partitions

  SYNOPSIS
    handle_opt_partitions()
    thd                      Thread object
    check_opt                Options
    operation                     Optimize/Analyze/Check/Repair flag

  RETURN VALUE
    >0                        Failure
    0                         Success
*/

int Partition_base::handle_opt_partitions(THD *thd, HA_CHECK_OPT *check_opt,
                                          enum_part_operation operation) {
  List_iterator<partition_element> part_it(m_part_info->partitions);
  uint num_parts = m_part_info->num_parts;
  uint num_subparts = m_part_info->num_subparts;
  uint i = 0;
  bool use_all_parts =
      !(thd->lex->alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION);
  int error;
  DBUG_ENTER("Partition_base::handle_opt_partitions");
  DBUG_PRINT("enter", ("operation= %u", operation));

  do {
    partition_element *part_elem = part_it++;
    /*
      when ALTER TABLE <CMD> PARTITION ...
      it should only do named [sub]partitions, otherwise all partitions
    */
    if (m_is_sub_partitioned) {
      List_iterator<partition_element> subpart_it(part_elem->subpartitions);
      partition_element *sub_elem;
      uint j = 0, part;
      do {
        sub_elem = subpart_it++;
        if (use_all_parts || part_elem->part_state == PART_ADMIN ||
            sub_elem->part_state == PART_ADMIN) {
          part = i * num_subparts + j;
          DBUG_PRINT("info", ("Optimize subpartition %u (%s)", part,
                              sub_elem->partition_name));
          if ((error = handle_opt_part(thd, check_opt, part, operation))) {
            /* print a line which partition the error belongs to */
            if (error != HA_ADMIN_NOT_IMPLEMENTED &&
                error != HA_ADMIN_ALREADY_DONE && error != HA_ADMIN_TRY_ALTER) {
              print_admin_msg(thd, MI_MAX_MSG_BUF, "error", table_share->db.str,
                              table->alias, opt_op_name[operation],
                              "Subpartition %s returned error",
                              sub_elem->partition_name);
            }
            /* reset part_state for the remaining partitions */
            do {
              if (sub_elem->part_state == PART_ADMIN)
                sub_elem->part_state = PART_NORMAL;
            } while ((sub_elem = subpart_it++));
            if (part_elem->part_state == PART_ADMIN)
              part_elem->part_state = PART_NORMAL;

            while ((part_elem = part_it++)) {
              List_iterator<partition_element> s_it(part_elem->subpartitions);
              while ((sub_elem = s_it++)) {
                if (sub_elem->part_state == PART_ADMIN)
                  sub_elem->part_state = PART_NORMAL;
              }
              if (part_elem->part_state == PART_ADMIN)
                part_elem->part_state = PART_NORMAL;
            }
            DBUG_RETURN(error);
          }
          sub_elem->part_state = PART_NORMAL;
        }
      } while (++j < num_subparts);
      part_elem->part_state = PART_NORMAL;
    } else {
      if (use_all_parts || part_elem->part_state == PART_ADMIN) {
        DBUG_PRINT("info", ("Optimize partition %u (%s)", i,
                            part_elem->partition_name));
        if ((error = handle_opt_part(thd, check_opt, i, operation))) {
          /* print a line which partition the error belongs to */
          if (error != HA_ADMIN_NOT_IMPLEMENTED &&
              error != HA_ADMIN_ALREADY_DONE && error != HA_ADMIN_TRY_ALTER) {
            print_admin_msg(thd, MI_MAX_MSG_BUF, "error", table_share->db.str,
                            table->alias, opt_op_name[operation],
                            "Partition %s returned error",
                            part_elem->partition_name);
          }
          /* reset part_state for the remaining partitions */
          do {
            if (part_elem->part_state == PART_ADMIN)
              part_elem->part_state = PART_NORMAL;
          } while ((part_elem = part_it++));
          DBUG_RETURN(error);
        }
      }
      part_elem->part_state = PART_NORMAL;
    }
  } while (++i < num_parts);
  DBUG_RETURN(false);
}

/**
  @brief Check and repair the table if neccesary

  @param thd    Thread object

  @retval true  Error/Not supported
  @retval false Success

  @note Called if open_table_from_share fails and ::is_crashed().
*/

bool Partition_base::check_and_repair(THD *thd) {
  handler **file = m_file;
  DBUG_ENTER("Partition_base::check_and_repair");

  do {
    if ((*file)->ha_check_and_repair(thd)) DBUG_RETURN(true);
  } while (*(++file));
  DBUG_RETURN(false);
}

/**
  @breif Check if the table can be automatically repaired

  @retval true  Can be auto repaired
  @retval false Cannot be auto repaired
*/

bool Partition_base::auto_repair() const {
  DBUG_ENTER("Partition_base::auto_repair");

  /*
    As long as we only support one storage engine per table,
    we can use the first partition for this function.
  */
  DBUG_RETURN(m_file[0]->auto_repair());
}

/**
  @breif Check if the table is crashed

  @retval true  Crashed
  @retval false Not crashed
*/

bool Partition_base::is_crashed() const {
  handler **file = m_file;
  DBUG_ENTER("Partition_base::is_crashed");

  do {
    if ((*file)->is_crashed()) DBUG_RETURN(true);
  } while (*(++file));
  DBUG_RETURN(false);
}

/**
  Prepare for creating new partitions during ALTER TABLE ... PARTITION.

  @param  num_partitions            Number of new partitions to be created.
  @param  only_create               True if only creating the partition
                                    (no open/lock is needed).
  @param  disable_non_uniq_indexes  True if non unique indexes are disabled.

  @return Operation status.
    @retval    0  Success.
    @retval != 0  Error code.
*/

int Partition_base::prepare_for_new_partitions(MEM_ROOT *mem_root,
                                               uint num_partitions) {
  size_t alloc_size = (num_partitions + 1) * sizeof(handler *);
  DBUG_ENTER("Partition_base::prepare_for_new_partition");
  m_new_file = (handler **)mem_root->Alloc(alloc_size);
  if (!m_new_file) {
    mem_alloc_error(alloc_size);
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  }
  memset(m_new_file, 0, alloc_size);
  size_t partition_name_size = (num_partitions) * sizeof(char *);
  m_new_partitions_name = (char **)mem_root->Alloc(partition_name_size);
  if (!m_new_partitions_name) {
    mem_alloc_error(partition_name_size);
    DBUG_RETURN(HA_ERR_OUT_OF_MEM);
  }
  memset(m_new_partitions_name, 0, partition_name_size);
  m_num_new_partitions = num_partitions;
  m_indexes_are_disabled = indexes_are_disabled();
  DBUG_RETURN(0);
}

/** Insert a row to the new partition.
  @param part_id  Partition to insert into.

  @return Operation status.
    @retval 0    Success
    @retval != 0 Error code
*/
int Partition_base::write_row_in_new_part(uint part_id) {
  int error;
  THD *thd = ha_thd();
  DBUG_ENTER("Partition_base::write_row_in_new_part");
  m_last_part = part_id;

  if (!m_new_file[part_id]) {
    /* Altered partition contains misplaced row. */
    m_err_rec = table->record[0];
    DBUG_RETURN(HA_ERR_ROW_IN_WRONG_PARTITION);
  }

  Disable_binlog_guard binlog_guard(thd);
  error = m_new_file[part_id]->ha_write_row(table->record[0]);
  DBUG_RETURN(error);
}

/*
  Close and unlock all created partitions.

  So they can be renamed and included in the altered table
  or deleted by the ddl-log in case of failure.
*/

void Partition_base::close_new_partitions(bool delete_new_partitions) {
  DBUG_ENTER("Partition_base::close_new_partitions");

  if (m_new_file) {
    for (uint i = 0; i < m_num_new_partitions; i++) {
      handler *file = m_new_file[i];

      if (file == nullptr) {
        /* Not a new partition, skip it. */
        continue;
      }
      file->ha_close();
      if (delete_new_partitions && m_new_partitions_name[i] != nullptr) {
        file->ha_delete_table(m_new_partitions_name[i], nullptr);
      }
    }

    m_new_file = nullptr;
  }
  DBUG_VOID_RETURN;
}

/*
  Update create info as part of ALTER TABLE

  SYNOPSIS
    update_create_info()
    create_info                   Create info from ALTER TABLE

  RETURN VALUE
    NONE

  DESCRIPTION
  Forward this handler call to the storage engine foreach
  partition handler.  The data_file_name for each partition may
  need to be reset if the tablespace was moved.  Use a dummy
  HA_CREATE_INFO structure and transfer necessary data.
*/

void Partition_base::update_create_info(HA_CREATE_INFO *create_info) {
  DBUG_ENTER("Partition_base::update_create_info");

  /*
    Fix for bug#38751, some engines needs info-calls in ALTER.
    Archive need this since it flushes in ::info.
    HA_STATUS_AUTO is optimized so it will not always be forwarded
    to all partitions, but HA_STATUS_VARIABLE will.
  */
  info(HA_STATUS_VARIABLE);

  info(HA_STATUS_AUTO);

  if (!(create_info->used_fields & HA_CREATE_USED_AUTO))
    create_info->auto_increment_value = stats.auto_increment_value;

  /*
    DATA DIRECTORY and INDEX DIRECTORY are never applied to the whole
    partitioned table, only its parts.
  */
  bool from_alter = (create_info->data_file_name == (const char *)-1);
  create_info->data_file_name = create_info->index_file_name = nullptr;

  /*
  We do not need to update the individual partition DATA DIRECTORY settings
  since they can be changed by ALTER TABLE ... REORGANIZE PARTITIONS.
  */
  if (from_alter) DBUG_VOID_RETURN;

  /*
    send Handler::update_create_info() to the storage engine for each
    partition that currently has a handler object.  Using a dummy
    HA_CREATE_INFO structure to collect DATA and INDEX DIRECTORYs.
  */

  List_iterator<partition_element> part_it(m_part_info->partitions);
  partition_element *part_elem, *sub_elem;
  uint num_subparts = m_part_info->num_subparts;
  uint num_parts =
      num_subparts ? m_file_tot_parts / num_subparts : m_file_tot_parts;
  HA_CREATE_INFO dummy_info;

  /*
  Since update_create_info() can be called from mysql_prepare_alter_table()
  when not all handlers are set up, we look for that condition first.
  If all handlers are not available, do not call update_create_info for any.
  */
  uint i, j, part;
  for (i = 0; i < num_parts; i++) {
    part_elem = part_it++;
    if (!part_elem) DBUG_VOID_RETURN;
    if (m_is_sub_partitioned) {
      List_iterator<partition_element> subpart_it(part_elem->subpartitions);
      for (j = 0; j < num_subparts; j++) {
        sub_elem = subpart_it++;
        if (!sub_elem) DBUG_VOID_RETURN;
        part = i * num_subparts + j;
        if (part >= m_file_tot_parts || !m_file[part]) DBUG_VOID_RETURN;
      }
    } else {
      if (!m_file[i]) DBUG_VOID_RETURN;
    }
  }
  part_it.rewind();

  for (i = 0; i < num_parts; i++) {
    part_elem = part_it++;
    DBUG_ASSERT(part_elem);
    if (m_is_sub_partitioned) {
      List_iterator<partition_element> subpart_it(part_elem->subpartitions);
      for (j = 0; j < num_subparts; j++) {
        sub_elem = subpart_it++;
        DBUG_ASSERT(sub_elem);
        part = i * num_subparts + j;
        DBUG_ASSERT(part < m_file_tot_parts && m_file[part]);
        if (ha_legacy_type(m_file[part]->ht) == DB_TYPE_INNODB) {
          dummy_info.data_file_name = dummy_info.index_file_name = nullptr;
          m_file[part]->update_create_info(&dummy_info);

          if (dummy_info.data_file_name || sub_elem->data_file_name) {
            sub_elem->data_file_name =
                const_cast<char *>(dummy_info.data_file_name);
          }
          if (dummy_info.index_file_name || sub_elem->index_file_name) {
            sub_elem->index_file_name =
                const_cast<char *>(dummy_info.index_file_name);
          }
        }
      }
    } else {
      DBUG_ASSERT(m_file[i]);
      if (ha_legacy_type(m_file[i]->ht) == DB_TYPE_INNODB) {
        dummy_info.data_file_name = dummy_info.index_file_name = nullptr;
        m_file[i]->update_create_info(&dummy_info);
        if (dummy_info.data_file_name || part_elem->data_file_name) {
          part_elem->data_file_name =
              const_cast<char *>(dummy_info.data_file_name);
        }
        if (dummy_info.index_file_name || part_elem->index_file_name) {
          part_elem->index_file_name =
              const_cast<char *>(dummy_info.index_file_name);
        }
      }
    }
  }
  DBUG_VOID_RETURN;
}

/**
  Change the internal TABLE_SHARE pointer

  @param table_arg    TABLE object
  @param share        New share to use

  @note Is used in error handling in ha_delete_table.
  All handlers should exist (lock_partitions should not be used)
*/

void Partition_base::change_table_ptr(TABLE *table_arg, TABLE_SHARE *share) {
  handler **file_array;
  table = table_arg;
  table_share = share;
  /*
    m_file can be nullptr when using an old cached table in DROP TABLE, when the
    table just has REMOVED PARTITIONING, see Bug#42438
  */
  if (m_file) {
    file_array = m_file;
    DBUG_ASSERT(*file_array);
    do {
      (*file_array)->change_table_ptr(table_arg, share);
    } while (*(++file_array));
  }
}

/**
  Handle delete and rename table

    @param from         Full path of old table
    @param to           Full path of new table

  @return Operation status
    @retval >0  Error
    @retval 0   Success

  @note  Common routine to handle delete_table and rename_table.
  The routine uses the partition handler file to get the
  names of the partition instances. Both these routines
  are called after creating the handler without table
  object and thus the file is needed to discover the
  names of the partitions and the underlying storage engines.
*/

int Partition_base::del_ren_table(const char *from, const char *to,
                                  const dd::Table *table_def_from,
                                  dd::Table *table_def_to) {
  int error = 0;
  int save_error = 0;
  const char *from_path;
  const char *to_path = nullptr;
  char from_lc_buff[FN_REFLEN];
  char to_lc_buff[FN_REFLEN];
  std::unique_ptr<handler, Destroy_only<handler>> file(
      get_file_handler(nullptr, ha_thd()->mem_root));
  /*
    Since Partition_base has HA_FILE_BASED, it must alter underlying table names
    if they do not have HA_FILE_BASED and lower_case_table_names == 2.
    See Bug#37402, for Mac OS X.
    The appended #P#<partname>[#SP#<subpartname>] will remain in current case.
    Using the first partitions handler, since mixing handlers is not allowed.
  */
  from_path = get_canonical_filename(file.get(), from, from_lc_buff);
  if (to != nullptr)
    to_path = get_canonical_filename(file.get(), to, to_lc_buff);

  std::vector<std::string> from_names;
  std::vector<std::string> to_names;

  for (const dd::Partition *dd_part : table_def_from->leaf_partitions()) {
    char name_buff[FN_REFLEN];
    part_name(name_buff, from_path,
              dd_part->parent() ? dd_part->parent()->name().c_str() : nullptr,
              dd_part->name().c_str());
    from_names.push_back(name_buff);
  }

  if (to)
    for (const dd::Partition *dd_part :
         const_cast<const dd::Table *>(table_def_to)->leaf_partitions()) {
      char name_buff[FN_REFLEN];
      part_name(name_buff, to_path,
                dd_part->parent() ? dd_part->parent()->name().c_str() : nullptr,
                dd_part->name().c_str());
      to_names.push_back(name_buff);
    }

  auto to_name = to_names.begin();

  for (const std::string &from_name : from_names) {
    if (!to) {
      error = file->ha_delete_table(from_name.c_str(), table_def_from);
      if (error) save_error = error;
    } else {
      // Currently neither TokuDB nor RocksDB process dd::Table* arguments,
      // so it's ok to pass nullptr as the last two arguments of the function.
      // But if some of the SE's implements DD-specific functionality,
      // it's necessary to modify the following code to pass
      // reasonable arguments. See also ha_innopart::rename_table()
      // to understand how to modify *table_def_to.
      error = file->ha_rename_table(from_name.c_str(), (to_name++)->c_str(),
                                    nullptr, nullptr);

      if (error) goto rename_error;
    }
  }

  return save_error;

rename_error:
  to_name = to_names.begin();
  for (const std::string &from_name : from_names) {
    // Currently neither TokuDB nor RocksDB process dd::Table* arguments,
    // so it's ok to pass nullptr as the last two arguments of the function.
    // But if some of the SE's implements DD-specific functionality,
    // it's necessary to modify the following code to pass
    // reasonable arguments. See also ha_innopart::rename_table()
    // to understand how to modify *table_def_to.
    file->ha_rename_table((to_name++)->c_str(), from_name.c_str(), nullptr,
                          nullptr);
  }
  return error;
}

/*
  Create underlying handler objects from partition info

  SYNOPSIS
    new_handlers_from_part_info()
    mem_root            Allocate memory through this

  RETURN VALUE
    true                  Error
    false                 Success
*/

bool Partition_base::new_handlers_from_part_info(MEM_ROOT *mem_root) {
  uint i, j, part_count;
  uint alloc_len = (m_tot_parts + 1) * sizeof(handler *);
  List_iterator_fast<partition_element> part_it(m_part_info->partitions);
  DBUG_ENTER("Partition_base::new_handlers_from_part_info");

  if (!(m_file = (handler **)mem_root->Alloc(alloc_len))) {
    mem_alloc_error(alloc_len);
    goto error_end;
  }
  m_file_tot_parts = m_tot_parts;
  memset(m_file, 0, alloc_len);
  DBUG_ASSERT(m_part_info->num_parts > 0);
  DBUG_ASSERT(m_part_info->num_parts == m_part_info->partitions.elements);

  i = 0;
  part_count = 0;
  /*
    Don't know the size of the underlying storage engine, invent a number of
    bytes allocated for error message if allocation fails
  */
  do {
    if (m_is_sub_partitioned) {
      for (j = 0; j < m_part_info->num_subparts; j++) {
        if (!(m_file[part_count++] = get_file_handler(table_share, mem_root)))
          goto error;
      }
    } else {
      if (!(m_file[part_count++] = get_file_handler(table_share, mem_root)))
        goto error;
    }
  } while (++i < m_part_info->num_parts);
  DBUG_RETURN(false);
error:
  mem_alloc_error(sizeof(handler));
error_end:
  DBUG_RETURN(true);
}

/****************************************************************************
                MODULE open/close object
****************************************************************************/

/**
  Set Handler_share pointer.

  @param ha_share_arg  Where to store/retrieve the Partitioning_share pointer
                       to be shared by all instances of the same table.

  @note: the function can be called before partition info is set, but
         Handler_share must be set for each partittion file in m_file,
         that is why the initial function was splitted into two functions:
         set_ha_share_ref() and init_part_share(). The latter is invoked after
         m_file initialization.

  @return Operation status
    @retval true  Failure
    @retval false Sucess
*/

bool Partition_base::set_ha_share_ref(Handler_share **ha_share_arg) {
  DBUG_ENTER("Partition_base::set_ha_share_ref");
  DBUG_ASSERT(!part_share);
  DBUG_ASSERT(table_share);

  if (handler::set_ha_share_ref(ha_share_arg)) DBUG_RETURN(true);
  DBUG_RETURN(false);
}

/**
  Allocate Handler_share pointers for each partition and set those.

  @return Operation status
    @retval true  Failure
    @retval false Sucess
*/

bool Partition_base::init_part_share() {
  DBUG_ENTER("Partition_base::init_part_share");
  DBUG_ASSERT(!part_share);
  DBUG_ASSERT(table_share);
  DBUG_ASSERT(m_tot_parts);

  if (!(part_share = get_share())) DBUG_RETURN(true);
  DBUG_ASSERT(part_share->partitions_share_refs);
  DBUG_ASSERT(part_share->partitions_share_refs->num_parts >= m_tot_parts);
  Handler_share **ha_shares = part_share->partitions_share_refs->ha_shares;
  for (uint i = 0; i < m_tot_parts; i++) {
    if (m_file[i]->set_ha_share_ref(&ha_shares[i])) DBUG_RETURN(true);
  }
  DBUG_RETURN(false);
}

/**
  Get the PARTITION_SHARE for the table.

  @return Operation status
    @retval true   Error
    @retval false  Success

  @note Gets or initializes the Partition_base_share object used by
  partitioning. The Partition_base_share is used for handling the auto_increment
  etc.
*/

Partition_base_share *Partition_base::get_share() {
  Partition_base_share *tmp_share;
  DBUG_ENTER("Partition_base::get_share");
  DBUG_ASSERT(table_share);

  lock_shared_ha_data();
  if (!(tmp_share = static_cast<Partition_base_share *>(get_ha_share_ptr()))) {
    tmp_share = new Partition_base_share;
    if (!tmp_share) goto err;
    if (tmp_share->init(m_tot_parts)) {
      delete tmp_share;
      tmp_share = nullptr;
      goto err;
    }
    if (table && table->found_next_number_field &&
        tmp_share->init_auto_inc_mutex(table_share)) {
      delete tmp_share;
      tmp_share = nullptr;
      goto err;
    }

    set_ha_share_ptr(static_cast<Partition_base_share *>(tmp_share));
  }
err:
  unlock_shared_ha_data();
  DBUG_RETURN(tmp_share);
}

/**
  Helper function for freeing all internal bitmaps.
*/

void Partition_base::free_partition_bitmaps() {
  /* Initialize the bitmap we use to minimize ha_start_bulk_insert calls */
  bitmap_free(&m_bulk_insert_started);
  bitmap_free(&m_locked_partitions);
  bitmap_free(&m_partitions_to_reset);
}

/**
  Helper function for initializing all internal bitmaps.
*/

bool Partition_base::init_partition_bitmaps() {
  DBUG_ENTER("Partition_base::init_partition_bitmaps");
  /* Initialize the bitmap we use to minimize ha_start_bulk_insert calls */
  if (bitmap_init(&m_bulk_insert_started, nullptr, m_tot_parts + 1))
    DBUG_RETURN(true);
  bitmap_clear_all(&m_bulk_insert_started);

  /* Initialize the bitmap we use to keep track of locked partitions */
  if (bitmap_init(&m_locked_partitions, nullptr, m_tot_parts)) {
    bitmap_free(&m_bulk_insert_started);
    DBUG_RETURN(true);
  }
  bitmap_clear_all(&m_locked_partitions);

  /*
    Initialize the bitmap we use to keep track of partitions which may have
    something to reset in ha_reset().
  */
  if (bitmap_init(&m_partitions_to_reset, nullptr, m_tot_parts)) {
    bitmap_free(&m_bulk_insert_started);
    bitmap_free(&m_locked_partitions);
    DBUG_RETURN(true);
  }
  bitmap_clear_all(&m_partitions_to_reset);

  /* When Partition_base is cloned, both the clone and the original object
  share partition_info object (m_part_info). Do not reset the partition
  bitmaps. */
  if (!m_clone_base) {
    /* Initialize the bitmap for read/lock_partitions */
    if (m_part_info->set_partition_bitmaps(nullptr)) {
      free_partition_bitmaps();
      DBUG_RETURN(true);
    }
  }

  DBUG_RETURN(false);
}

/*
  Open handler object

  SYNOPSIS
    open()
    name                  Full path of table name
    mode                  Open mode flags
    test_if_locked        ?

  RETURN VALUE
    >0                    Error
    0                     Success

  DESCRIPTION
    Used for opening tables. The name will be the name of the file.
    A table is opened when it needs to be opened. For instance
    when a request comes in for a select on the table (tables are not
    open and closed for each request, they are cached).

    Called from handler.cc by handler::ha_open(). The server opens all tables
    by calling ha_open() which then calls the handler specific open().
*/
int Partition_base::open(const char *name, int mode, uint test_if_locked,
                         const dd::Table *table_def) {
  int error = HA_ERR_INITIALIZATION;
  handler **file;
  handler **clone_base_file = nullptr;
  ulonglong check_table_flags;
  DBUG_ENTER("Partition_base::open");

  DBUG_ASSERT(table->s == table_share);
  DBUG_ASSERT(m_part_info);
  ref_length = 0;
  m_mode = mode;

  MEM_ROOT *mem_root =
      m_clone_mem_root != nullptr ? m_clone_mem_root : &table->mem_root;

  /* The following functions must be called only after m_part_info set */
  if (initialize_partition(mem_root) || init_part_share() || init_with_fields())
    DBUG_RETURN(true);

  /* Check/update the partition share. */
  lock_shared_ha_data();
  if (part_share->populate_partition_name_hash(m_part_info)) {
    unlock_shared_ha_data();
    DBUG_RETURN(HA_ERR_INITIALIZATION);
  }
  if (!part_share->auto_inc_mutex && table->found_next_number_field) {
    if (part_share->init_auto_inc_mutex(table_share)) {
      unlock_shared_ha_data();
      DBUG_RETURN(HA_ERR_INITIALIZATION);
    }
  }
  unlock_shared_ha_data();

  if (open_partitioning(part_share)) {
    goto err;
  }
  DBUG_ASSERT(!m_file_tot_parts || m_file_tot_parts == m_tot_parts);
  if (!m_part_ids_sorted_by_num_of_records) {
    if (!(m_part_ids_sorted_by_num_of_records =
              (uint32 *)my_malloc(key_memory_Partition_base_part_ids,
                                  m_tot_parts * sizeof(uint32), MYF(MY_WME)))) {
      goto err;
    }
    uint32 i;
    /* Initialize it with all partition ids. */
    for (i = 0; i < m_tot_parts; i++)
      m_part_ids_sorted_by_num_of_records[i] = i;
  }

  if (init_partition_bitmaps()) {
    goto err;
  }

  DBUG_ASSERT(m_part_info);

  file = m_file;
  if (m_clone_base != nullptr) {
    clone_base_file = m_clone_base->m_file;
  }

  if (!foreach_partition([&](const partition_element *parent_part_elem,
                             const partition_element *part_elem) -> bool {
        char name_buff[FN_REFLEN];
        part_name(name_buff, name,
                  parent_part_elem ? parent_part_elem->partition_name : nullptr,
                  part_elem->partition_name);

        if (m_clone_base != nullptr) {
          uint ref_length = (*clone_base_file)->ref_length;
          (*file)->ref =
              (uchar *)m_clone_mem_root->Alloc(ALIGN_SIZE(ref_length) * 2);
        }

        if ((error = (*file)->ha_open(table, name_buff, mode, test_if_locked,
                                      table_def)))
          return false;

        ++file;
        if (m_clone_base != nullptr) {
          ++clone_base_file;
        }

        return true;
      }))
    goto err_handler;

  file = m_file;
  ref_length = (*file)->ref_length;
  check_table_flags =
      (((*file)->ha_table_flags() & ~(PARTITION_DISABLED_TABLE_FLAGS)) |
       (PARTITION_ENABLED_TABLE_FLAGS));
  while (*(++file)) {
    /* MyISAM can have smaller ref_length for partitions with MAX_ROWS set */
    ref_length = std::max(ref_length, ((*file)->ref_length));
    /*
      Verify that all partitions have the same set of table flags.
      Mask all flags that partitioning enables/disables.
    */
    if (check_table_flags !=
        (((*file)->ha_table_flags() & ~(PARTITION_DISABLED_TABLE_FLAGS)) |
         (PARTITION_ENABLED_TABLE_FLAGS))) {
      error = HA_ERR_INITIALIZATION;
      /* set file to last handler, so all of them are closed */
      file = &m_file[m_tot_parts - 1];
      goto err_handler;
    }
  }
  key_used_on_scan = m_file[0]->key_used_on_scan;
  implicit_emptied = m_file[0]->implicit_emptied;
  /*
    Add 2 bytes for partition id in position ref length.
    ref_length=max_in_all_partitions(ref_length) + PARTITION_BYTES_IN_POS
  */
  ref_length += PARTITION_BYTES_IN_POS;

  /*
    Some handlers update statistics as part of the open call. This will in
    some cases corrupt the statistics of the partition handler and thus
    to ensure we have correct statistics we call info from open after
    calling open on all individual handlers.
  */
  m_handler_status = handler_opened;
  if (m_part_info->part_expr)
    m_part_func_monotonicity_info =
        m_part_info->part_expr->get_monotonicity_info();
  else if (m_part_info->list_of_part_fields)
    m_part_func_monotonicity_info = MONOTONIC_STRICT_INCREASING;
  info(HA_STATUS_VARIABLE | HA_STATUS_CONST);
  DBUG_RETURN(0);

err_handler:
  DEBUG_SYNC(ha_thd(), "partition_open_error");
  while (file-- != m_file) (*file)->ha_close();
  free_partition_bitmaps();
err:
  close_partitioning();

  DBUG_RETURN(error);
}

/*
  Close handler object

  SYNOPSIS
    close()

  RETURN VALUE
    >0                   Error code
    0                    Success

  DESCRIPTION
    Called from sql_base.cc, sql_select.cc, and table.cc.
    In sql_select.cc it is only used to close up temporary tables or during
    the process where a temporary table is converted over to being a
    myisam table.
    For sql_base.cc look at close_data_tables().
*/

int Partition_base::close(void) {
  handler **file;
  DBUG_ENTER("Partition_base::close");

  DBUG_ASSERT(table->s == table_share);
  close_partitioning();
  free_partition_bitmaps();
  DBUG_ASSERT(m_part_info);
  file = m_file;

  do {
    (*file)->ha_close();
  } while (*(++file));

  close_new_partitions();

  m_handler_status = handler_closed;
  DBUG_RETURN(0);
}

/****************************************************************************
                MODULE start/end statement
****************************************************************************/
/*
  A number of methods to define various constants for the handler. In
  the case of the partition handler we need to use some max and min
  of the underlying handlers in most cases.
*/

/*
  Set external locks on table

  SYNOPSIS
    external_lock()
    thd                    Thread object
    lock_type              Type of external lock

  RETURN VALUE
    >0                   Error code
    0                    Success

  DESCRIPTION
    Originally this method was used to set locks on file level to enable
    several MySQL Servers to work on the same data. For transactional
    engines it has been "abused" to also mean start and end of statements
    to enable proper rollback of statements and transactions. When LOCK
    TABLES has been issued the start_stmt method takes over the role of
    indicating start of statement but in this case there is no end of
    statement indicator(?).

    Called from lock.cc by lock_external() and unlock_external(). Also called
    from sql_table.cc by copy_data_between_tables().
*/

int Partition_base::external_lock(THD *thd, int lock_type) {
  uint error;
  uint i, first_used_partition;
  MY_BITMAP *used_partitions;
  DBUG_ENTER("Partition_base::external_lock");

  DBUG_ASSERT(!m_auto_increment_lock && !m_auto_increment_safe_stmt_log_lock);

  if (lock_type == F_UNLCK)
    used_partitions = &m_locked_partitions;
  else
    used_partitions = &(m_part_info->lock_partitions);

  first_used_partition = bitmap_get_first_set(used_partitions);

  for (i = first_used_partition; i < m_tot_parts;
       i = bitmap_get_next_set(used_partitions, i)) {
    DBUG_PRINT("info", ("external_lock(thd, %d) part %d", lock_type, i));
    if ((error = m_file[i]->ha_external_lock(thd, lock_type))) {
      if (lock_type != F_UNLCK) goto err_handler;
    }
    DBUG_PRINT("info", ("external_lock part %u lock %d", i, lock_type));
    if (lock_type != F_UNLCK) bitmap_set_bit(&m_locked_partitions, i);
  }
  if (lock_type == F_UNLCK) {
    bitmap_clear_all(used_partitions);
  } else {
    /* Add touched partitions to be included in reset(). */
    bitmap_union(&m_partitions_to_reset, used_partitions);
  }

  DBUG_RETURN(0);

err_handler:
  uint j;
  for (j = first_used_partition; j < i;
       j = bitmap_get_next_set(&m_locked_partitions, j)) {
    (void)m_file[j]->ha_external_lock(thd, F_UNLCK);
  }
  bitmap_clear_all(&m_locked_partitions);
  DBUG_RETURN(error);
}

/*
  Get the lock(s) for the table and perform conversion of locks if needed

  SYNOPSIS
    store_lock()
    thd                   Thread object
    to                    Lock object array
    lock_type             Table lock type

  RETURN VALUE
    >0                   Error code
    0                    Success

  DESCRIPTION
    The idea with handler::store_lock() is the following:

    The statement decided which locks we should need for the table
    for updates/deletes/inserts we get WRITE locks, for SELECT... we get
    read locks.

    Before adding the lock into the table lock handler (see thr_lock.c)
    mysqld calls store lock with the requested locks.  Store lock can now
    modify a write lock to a read lock (or some other lock), ignore the
    lock (if we don't want to use MySQL table locks at all) or add locks
    for many tables (like we do when we are using a MERGE handler).

    When releasing locks, store_lock() is also called. In this case one
    usually doesn't have to do anything.

    store_lock is called when holding a global mutex to ensure that only
    one thread at a time changes the locking information of tables.

    In some exceptional cases MySQL may send a request for a TL_IGNORE;
    This means that we are requesting the same lock as last time and this
    should also be ignored. (This may happen when someone does a flush
    table when we have opened a part of the tables, in which case mysqld
    closes and reopens the tables and tries to get the same locks as last
    time).  In the future we will probably try to remove this.

    Called from lock.cc by get_lock_data().
*/

THR_LOCK_DATA **Partition_base::store_lock(THD *thd, THR_LOCK_DATA **to,
                                           enum thr_lock_type lock_type) {
  uint i;
  DBUG_ENTER("Partition_base::store_lock");
  DBUG_ASSERT(thd == ha_thd());

  /*
    This can be called from get_lock_data() in mysql_lock_abort_for_thread(),
    even when thd != table->in_use. In that case don't use partition pruning,
    but use all partitions instead to avoid using another threads structures.
  */
  if (thd != table->in_use) {
    for (i = 0; i < m_tot_parts; i++)
      to = m_file[i]->store_lock(thd, to, lock_type);
  } else {
    for (i = bitmap_get_first_set(&(m_part_info->lock_partitions));
         i < m_tot_parts;
         i = bitmap_get_next_set(&m_part_info->lock_partitions, i)) {
      DBUG_PRINT("info", ("store lock %d iteration", i));
      to = m_file[i]->store_lock(thd, to, lock_type);
    }
  }
  DBUG_RETURN(to);
}

/*
  Start a statement when table is locked

  SYNOPSIS
    start_stmt()
    thd                  Thread object
    lock_type            Type of external lock

  RETURN VALUE
    >0                   Error code
    0                    Success

  DESCRIPTION
    This method is called instead of external lock when the table is locked
    before the statement is executed.
*/

int Partition_base::start_stmt(THD *thd, thr_lock_type lock_type) {
  int error = 0;
  uint i;
  /* Assert that read_partitions is included in lock_partitions */
  DBUG_ASSERT(bitmap_is_subset(&m_part_info->read_partitions,
                               &m_part_info->lock_partitions));
  /*
    m_locked_partitions is set in previous external_lock/LOCK TABLES.
    Current statement's lock requests must not include any partitions
    not previously locked.
  */
  DBUG_ASSERT(
      bitmap_is_subset(&m_part_info->lock_partitions, &m_locked_partitions));
  DBUG_ENTER("Partition_base::start_stmt");

  for (i = bitmap_get_first_set(&(m_part_info->lock_partitions));
       i < m_tot_parts;
       i = bitmap_get_next_set(&m_part_info->lock_partitions, i)) {
    if ((error = m_file[i]->start_stmt(thd, lock_type))) break;
    /* Add partition to be called in reset(). */
    bitmap_set_bit(&m_partitions_to_reset, i);
  }
  DBUG_RETURN(error);
}

/**
  Get number of lock objects returned in store_lock

  @returns Number of locks returned in call to store_lock

  @desc
    Returns the number of store locks needed in call to store lock.
    We return number of partitions we will lock multiplied with number of
    locks needed by each partition. Assists the above functions in allocating
    sufficient space for lock structures.
*/

uint Partition_base::lock_count() const {
  DBUG_ENTER("Partition_base::lock_count");
  /*
    The caller want to know the upper bound, to allocate enough memory.
    There is no performance lost if we simply return maximum number locks
    needed, only some minor over allocation of memory in get_lock_data().

    Also notice that this may be called for another thread != table->in_use,
    when mysql_lock_abort_for_thread() is called. So this is more safe, then
    using number of partitions after pruning.
  */
  if (m_file && m_file[0]) DBUG_RETURN(m_tot_parts * m_file[0]->lock_count());
  DBUG_RETURN(0);
}

/*
  Unlock last accessed row

  SYNOPSIS
    unlock_row()

  RETURN VALUE
    NONE

  DESCRIPTION
    Record currently processed was not in the result set of the statement
    and is thus unlocked. Used for UPDATE and DELETE queries.
*/

void Partition_base::unlock_row() {
  DBUG_ENTER("Partition_base::unlock_row");
  m_file[m_last_part]->unlock_row();
  DBUG_VOID_RETURN;
}

/**
  Check if semi consistent read was used

  SYNOPSIS
    was_semi_consistent_read()

  RETURN VALUE
    true   Previous read was a semi consistent read
    false  Previous read was not a semi consistent read

  DESCRIPTION
    See handler.h:
    In an UPDATE or DELETE, if the row under the cursor was locked by another
    transaction, and the engine used an optimistic read of the last
    committed row value under the cursor, then the engine returns 1 from this
    function. MySQL must NOT try to update this optimistic value. If the
    optimistic value does not match the WHERE condition, MySQL can decide to
    skip over this row. Currently only works for InnoDB. This can be used to
    avoid unnecessary lock waits.

    If this method returns nonzero, it will also signal the storage
    engine that the next read will be a locking re-read of the row.
*/
bool Partition_base::was_semi_consistent_read() {
  DBUG_ENTER("Partition_base::was_semi_consistent_read");
  DBUG_ASSERT(m_last_part < m_tot_parts &&
              m_part_info->is_partition_used(m_last_part));
  DBUG_RETURN(m_file[m_last_part]->was_semi_consistent_read());
}

/**
  Use semi consistent read if possible

  SYNOPSIS
    try_semi_consistent_read()
    yes   Turn on semi consistent read

  RETURN VALUE
    NONE

  DESCRIPTION
    See handler.h:
    Tell the engine whether it should avoid unnecessary lock waits.
    If yes, in an UPDATE or DELETE, if the row under the cursor was locked
    by another transaction, the engine may try an optimistic read of
    the last committed row value under the cursor.
    Note: prune_partitions are already called before this call, so using
    pruning is OK.
*/
void Partition_base::try_semi_consistent_read(bool yes) {
  uint i;
  DBUG_ENTER("Partition_base::try_semi_consistent_read");

  i = m_part_info->get_first_used_partition();
  DBUG_ASSERT(i != MY_BIT_NONE);
  for (; i < m_tot_parts; i = m_part_info->get_next_used_partition(i)) {
    m_file[i]->try_semi_consistent_read(yes);
  }
  DBUG_VOID_RETURN;
}

/****************************************************************************
                MODULE change record
****************************************************************************/

/** Insert a row to the partition.
  @param part_id  Partition to insert into.
  @param buf      The row in MySQL Row Format.

  @return Operation status.
    @retval 0    Success
    @retval != 0 Error code
*/
int Partition_base::write_row_in_part(uint part_id, uchar *buf) {
  int error;
  THD *thd = ha_thd();
  DBUG_ENTER("Partition_base::write_row_in_part");
  m_last_part = part_id;
  start_part_bulk_insert(thd, part_id);

  Disable_binlog_guard binlog_guard(thd);
  error = m_file[part_id]->ha_write_row(buf);
  DBUG_RETURN(error);
}

int Partition_base::update_row_in_part(uint part_id, const uchar *old_data,
                                       uchar *new_data) {
  int error;
  THD *thd = ha_thd();
  DBUG_ENTER("Partition_base::update_row_in_part");
  start_part_bulk_insert(thd, part_id);

  Disable_binlog_guard binlog_guard(thd);
  error = m_file[part_id]->ha_update_row(old_data, new_data);
  DBUG_RETURN(error);
}

/**
  Delete an existing row in the partition.

  This will delete a row. buf will contain a copy of the row to be deleted.
  The server will call this right after the current row has been read
  (from either a previous rnd_xxx() or index_xxx() call).
  If you keep a pointer to the last row or can access a primary key it will
  make doing the deletion quite a bit easier.
  Keep in mind that the server does no guarantee consecutive deletions.
  ORDER BY clauses can be used.

  buf is either record[0] or record[1]

  @param part_id  The partition to delete the row from.
  @param buf      The record in MySQL Row Format.

  @return Operation status.
    @retval 0    Success
    @retval != 0 Error code
*/

int Partition_base::delete_row_in_part(uint part_id, const uchar *buf) {
  int error;
  THD *thd = ha_thd();
  DBUG_ENTER("Partition_base::delete_row_in_part");

  m_last_part = part_id;
  /* Do not replicate low level changes, already registered in ha_* wrapper. */
  Disable_binlog_guard binlog_guard(thd);
  error = m_file[part_id]->ha_delete_row(buf);
  DBUG_RETURN(error);
}

/*
  Delete all rows in a table

  SYNOPSIS
    delete_all_rows()

  RETURN VALUE
    >0                       Error Code
    0                        Success

  DESCRIPTION
    Used to delete all rows in a table. Both for cases of truncate and
    for cases where the optimizer realizes that all rows will be
    removed as a result of a SQL statement.

    Called from item_sum.cc by Item_func_group_concat::clear(),
    Item_sum_count_distinct::clear(), and Item_func_group_concat::clear().
    Called from sql_delete.cc by mysql_delete().
    Called from sql_select.cc by JOIN::reset().
    Called from sql_union.cc by st_select_lex_unit::exec().
*/

int Partition_base::delete_all_rows() {
  int error;
  uint i;
  DBUG_ENTER("Partition_base::delete_all_rows");

  for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
       i = m_part_info->get_next_used_partition(i)) {
    /* Can be pruned, like DELETE FROM t PARTITION (pX) */
    if ((error = m_file[i]->ha_delete_all_rows())) DBUG_RETURN(error);
  }
  DBUG_RETURN(0);
}

/**
  Manually truncate the table.

  @retval  0    Success.
  @retval  > 0  Error code.
*/

int Partition_base::truncate(dd::Table *table_def) {
  int error;
  handler **file;
  DBUG_ENTER("Partition_base::truncate");

  /*
    TRUNCATE also means resetting auto_increment. Hence, reset
    it so that it will be initialized again at the next use.
  */
  if (table->found_next_number_field) {
    // TODO: Create Partition_helper::reset_auto_inc().
    lock_auto_increment();
    part_share->next_auto_inc_val = 0;
    part_share->auto_inc_initialized = false;
    unlock_auto_increment();
  }

  file = m_file;
  do {
    if ((error = (*file)->ha_truncate(table_def))) DBUG_RETURN(error);
  } while (*(++file));
  DBUG_RETURN(0);
}

/**
  Truncate a set of specific partitions.

  @remark Auto increment value will be truncated in that partition as well!

  ALTER TABLE t TRUNCATE PARTITION ...
*/
int Partition_base::truncate_partition_low(dd::Table *table_def) {
  int error = 0;
  List_iterator<partition_element> part_it(m_part_info->partitions);
  uint i = 0;
  DBUG_ENTER("Partition_base::truncate_partition");

  /*
    TRUNCATE also means resetting auto_increment. Hence, reset
    it so that it will be initialized again at the next use.
  */
  if (table->found_next_number_field) {
    lock_auto_increment();
    part_share->next_auto_inc_val = 0;
    part_share->auto_inc_initialized = false;
    unlock_auto_increment();
  }

  for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
       i = m_part_info->get_next_used_partition(i)) {
    DBUG_PRINT("info", ("truncate partition %u", i));
    if ((error = m_file[i]->ha_truncate(table_def))) break;
  }
  if (error) {
    /* Reset to PART_NORMAL. */
    set_all_part_state(m_part_info, PART_NORMAL);
  }
  DBUG_RETURN(error);
}

/*
  Start a large batch of insert rows

  SYNOPSIS
    start_bulk_insert()
    rows                  Number of rows to insert

  RETURN VALUE
    NONE

  DESCRIPTION
    rows == 0 means we will probably insert many rows
*/
void Partition_base::start_bulk_insert(ha_rows /*rows*/) {
  DBUG_ENTER("Partition_base::start_bulk_insert");

  m_bulk_inserted_rows = 0;
  bitmap_clear_all(&m_bulk_insert_started);
  /* use the last bit for marking if bulk_insert_started was called */
  bitmap_set_bit(&m_bulk_insert_started, m_tot_parts);
  DBUG_VOID_RETURN;
}

/*
  Check if start_bulk_insert has been called for this partition,
  if not, call it and mark it called
*/
void Partition_base::start_part_bulk_insert(THD *thd, uint part_id) {
  long old_buffer_size;
  if (!bitmap_is_set(&m_bulk_insert_started, part_id) &&
      bitmap_is_set(&m_bulk_insert_started, m_tot_parts)) {
    DBUG_ASSERT(bitmap_is_set(&(m_part_info->lock_partitions), part_id));
    old_buffer_size = thd->variables.read_buff_size;
    /* Update read_buffer_size for this partition */
    thd->variables.read_buff_size = estimate_read_buffer_size(old_buffer_size);
    m_file[part_id]->ha_start_bulk_insert(guess_bulk_insert_rows());
    bitmap_set_bit(&m_bulk_insert_started, part_id);
    thd->variables.read_buff_size = old_buffer_size;
  }
  m_bulk_inserted_rows++;
}

/*
  Estimate the read buffer size for each partition.
  SYNOPSIS
    Partition_base::estimate_read_buffer_size()
    original_size  read buffer size originally set for the server
  RETURN VALUE
    estimated buffer size.
  DESCRIPTION
    If the estimated number of rows to insert is less than 10 (but not 0)
    the new buffer size is same as original buffer size.
    In case of first partition of when partition function is monotonic
    new buffer size is same as the original buffer size.
    For rest of the partition total buffer of 10*original_size is divided
    equally if number of partition is more than 10 other wise each partition
    will be allowed to use original buffer size.
*/
long Partition_base::estimate_read_buffer_size(long original_size) {
  /*
    If number of rows to insert is less than 10, but not 0,
    return original buffer size.
  */
  if (estimation_rows_to_insert && (estimation_rows_to_insert < 10))
    return (original_size);
  /*
    If first insert/partition and monotonic partition function,
    allow using buffer size originally set.
   */
  if (!m_bulk_inserted_rows && m_part_func_monotonicity_info != NON_MONOTONIC &&
      m_tot_parts > 1)
    return original_size;
  /*
    Allow total buffer used in all partition to go up to 10*read_buffer_size.
    11*read_buffer_size in case of monotonic partition function.
  */

  if (m_tot_parts < 10) return original_size;
  return (original_size * 10 / m_tot_parts);
}

/*
  Try to predict the number of inserts into this partition.

  If less than 10 rows (including 0 which means Unknown)
    just give that as a guess
  If monotonic partitioning function was used
    guess that 50 % of the inserts goes to the first partition
  For all other cases, guess on equal distribution between the partitions
*/
ha_rows Partition_base::guess_bulk_insert_rows() {
  DBUG_ENTER("guess_bulk_insert_rows");

  if (estimation_rows_to_insert < 10) DBUG_RETURN(estimation_rows_to_insert);

  /* If first insert/partition and monotonic partition function, guess 50%.  */
  if (!m_bulk_inserted_rows && m_part_func_monotonicity_info != NON_MONOTONIC &&
      m_tot_parts > 1)
    DBUG_RETURN(estimation_rows_to_insert / 2);

  /* Else guess on equal distribution (+1 is to avoid returning 0/Unknown) */
  if (m_bulk_inserted_rows < estimation_rows_to_insert)
    DBUG_RETURN(
        ((estimation_rows_to_insert - m_bulk_inserted_rows) / m_tot_parts) + 1);
  /* The estimation was wrong, must say 'Unknown' */
  DBUG_RETURN(0);
}

/**
  Finish a large batch of insert rows.

  @return Operation status.
    @retval     0 Success
    @retval  != 0 Error code
*/

int Partition_base::end_bulk_insert() {
  int error = 0;
  uint i;
  DBUG_ENTER("Partition_base::end_bulk_insert");

  if (!bitmap_is_set(&m_bulk_insert_started, m_tot_parts)) {
    DBUG_ASSERT(0);
    DBUG_RETURN(error);
  }

  for (i = bitmap_get_first_set(&m_bulk_insert_started); i < m_tot_parts;
       i = bitmap_get_next_set(&m_bulk_insert_started, i)) {
    int tmp;
    if ((tmp = m_file[i]->ha_end_bulk_insert())) error = tmp;
  }
  bitmap_clear_all(&m_bulk_insert_started);

  DBUG_EXECUTE_IF("ha_partition_end_bulk_insert_fail", {
    error = 1;
    set_my_errno(EPERM);
  });
  DBUG_RETURN(error);
}

/****************************************************************************
                MODULE full table scan
****************************************************************************/

/**
  Initialize partition for random reads.

  rnd_init() is called when the server wants the storage engine to do a
  table scan or when the server wants to access data through rnd_pos.

  When scan is used we will scan one handler partition at a time.
  When preparing for rnd_pos we will initialize all handler partitions.
  No extra cache handling is needed when scanning is not performed.

  Before initializing we will call rnd_end to ensure that we clean up from
  any previous incarnation of a table scan.

  @param part_id  partition to initialize.
  @param scan     false for initialize for random reads through rnd_pos()
                  true for initialize for random scan through rnd_next().

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::rnd_init_in_part(uint part_id, bool scan) {
  return m_file[part_id]->ha_rnd_init(scan);
}

/**
  End of a partition scan.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::rnd_end_in_part(uint part_id,
                                    bool scan MY_ATTRIBUTE((unused))) {
  return m_file[part_id]->ha_rnd_end();
}

/**
  Read next row during full partition scan (scan in random row order).

  This is called for each row of the table scan. When you run out of records
  you should return HA_ERR_END_OF_FILE.
  The Field structure for the table is the key to getting data into buf
  in a manner that will allow the server to understand it.

  @param[in]     part_id  Partition to read from.
  @param[in,out] buf      buffer that should be filled with data.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::rnd_next_in_part(uint part_id, uchar *buf) {
  return m_file[part_id]->ha_rnd_next(buf);
}

/**
  Save position of current row.

  position() is called after each call to rnd_next() if the data needs
  to be ordered.

  The server uses ref to store data. ref_length in the above case is
  the size needed to store current_position. ref is just a byte array
  that the server will maintain. If you are using offsets to mark rows, then
  current_position should be the offset. If it is a primary key like in
  InnoDB, then it needs to be a primary key.

  @param record  Current record in MySQL Row Format.

  @note m_last_part must be set (normally done by
  Partition_helper::return_top_record()).
*/

void Partition_base::position_in_last_part(uchar *ref_value,
                                           const uchar *record) {
  handler *file = m_file[m_last_part];
  file->position(record);
  memcpy(ref_value, file->ref, file->ref_length);
  /* MyISAM partitions can have different ref_length depending on MAX_ROWS! */
  uint pad_length = ref_length - PARTITION_BYTES_IN_POS - file->ref_length;
  if (pad_length)
    memset((ref_value + PARTITION_BYTES_IN_POS + file->ref_length), 0,
           pad_length);
}

/**
  Read row from partition using position.

  This is like rnd_next, but you are given a position to use to determine
  the row. The position will be pointing to data of length handler::ref_length
  that handler::ref was set by position(record). Tables clustered on primary
  key usually use the full primary key as reference (like InnoDB). Heap based
  tables usually returns offset in heap file (like MyISAM).

  @param[in]     part_id  Partition to read from.
  @param[in,out] buf      Buffer to fill with record in MySQL format.
  @param[in]     pos      Position (data pointed to from ::ref) from position().

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::rnd_pos_in_part(uint part_id, uchar *buf, uchar *pos) {
  return m_file[part_id]->ha_rnd_pos(buf, pos);
}

/****************************************************************************
                MODULE index scan
****************************************************************************/
/*
  Positions an index cursor to the index specified in the handle. Fetches the
  row if available. If the key value is null, begin at the first key of the
  index.
*/

/** Compare key and rowid.
  Helper function for sorting records in the priority queue.
  a/b points to table->record[0] rows which must have the
  key fields set. The bytes before a and b store the handler::ref.
  This is used for comparing/sorting rows first according to
  KEY and if same KEY, by handler::ref (rowid).

  @param key_info  Null terminated array of index information
  @param a         Pointer to record+ref in first record
  @param b         Pointer to record+ref in second record

  @return Return value is SIGN(first_rec - second_rec)
    @retval  0                  Keys are equal
    @retval -1                  second_rec is greater than first_rec
    @retval +1                  first_rec is greater than second_rec
*/

static int key_and_ref_cmp(KEY **key_info, uchar *a, uchar *b) {
  int cmp = key_rec_cmp(key_info, a, b);
  if (cmp) return cmp;
  /*
    We must compare by handler::ref, which is added before the record,
    in the priority queue.
  */
  KEY **key = key_info;
  uint ref_length = (*key)->table->file->ref_length;
  return (*key)->table->file->cmp_ref(a - ref_length, b - ref_length);
}

/**
  Initialize partition before start of index scan.

  @param part    Partition to initialize the index in.
  @param inx     Index number.
  @param sorted  Is rows to be returned in sorted order.

  @return Operation status
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_init_in_part(uint part, uint keynr, bool sorted) {
  return m_file[part]->ha_index_init(keynr, sorted);
}

/**
  End of index scan in a partition.

  index_end_in_part is called at the end of an index scan to clean up any
  things needed to clean up.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_end_in_part(uint part) {
  return m_file[part]->ha_index_end();
}

/**
  Read one record in an index scan and start an index scan in one partition.

  index_read_map_in_part starts a new index scan using a start key.
  index_read_map_in_part can be restarted without calling index_end on
  the previous index scan and without calling index_init.
  In this case the index_read_map_in_part is on the same index as the previous
  index_scan. This is particularly used in conjunction with multi read ranges.

  @param[in]     part         Partition to read from.
  @param[in,out] buf          Read row in MySQL Row Format
  @param[in]     key          Key parts in consecutive order
  @param[in]     keypart_map  Which part of key is used
  @param[in]     find_flag    What type of key condition is used

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_read_map_in_part(uint part, uchar *buf,
                                           const uchar *key,
                                           key_part_map keypart_map,
                                           enum ha_rkey_function find_flag) {
  return m_file[part]->ha_index_read_map(buf, key, keypart_map, find_flag);
}

/**
  Start an index scan from leftmost record and return first record.

  index_first() asks for the first key in the index.
  This is similar to index_read except that there is no start key since
  the scan starts from the leftmost entry and proceeds forward with
  index_next.

  @param[in]     part  Partition to read from.
  @param[in,out] buf   Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_first_in_part(uint part, uchar *buf) {
  return m_file[part]->ha_index_first(buf);
}

/**
  Start an index scan from rightmost record and return first record.

  index_last() asks for the last key in the index.
  This is similar to index_read except that there is no start key since
  the scan starts from the rightmost entry and proceeds forward with
  index_prev.

  @param[in]     part  Partition to read from.
  @param[in,out] buf   Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_last_in_part(uint part, uchar *buf) {
  return m_file[part]->ha_index_last(buf);
}

/**
  Read last using key.

  This is used in join_read_last_key to optimize away an ORDER BY.
  Can only be used on indexes supporting HA_READ_ORDER.

  @param[in,out] buf          Read row in MySQL Row Format
  @param[in]     key          Key
  @param[in]     keypart_map  Which part of key is used

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_read_last_map_in_part(uint part, uchar *buf,
                                                const uchar *key,
                                                key_part_map keypart_map) {
  return m_file[part]->ha_index_read_last_map(buf, key, keypart_map);
}

/**
  Read index by key and keymap in a partition.

  @param[in]     part         Index to read from
  @param[in,out] buf          Read row in MySQL Row Format
  @param[in]     index        Index to read from
  @param[in]     key          Key
  @param[in]     keypart_map  Which part of key is used
  @param[in]     find_flag    Direction/how to search.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_read_idx_map_in_part(
    uint part, uchar *buf, uint index, const uchar *key,
    key_part_map keypart_map, enum ha_rkey_function find_flag) {
  return m_file[part]->ha_index_read_idx_map(buf, index, key, keypart_map,
                                             find_flag);
}

/**
  Read next record in a forward index scan.

  Used to read forward through the index (left to right, low to high).

  @param[in]     part  Partition to read from.
  @param[in,out] buf   Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_next_in_part(uint part, uchar *buf) {
  return m_file[part]->ha_index_next(buf);
}

/**
  Read next same record in partition.

  This routine is used to read the next but only if the key is the same
  as supplied in the call.

  @param[in]     part    Partition to read from.
  @param[in,out] buf     Read row in MySQL Row Format.
  @param[in]     key     Key.
  @param[in]     keylen  Length of key.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_next_same_in_part(uint part, uchar *buf,
                                            const uchar *key, uint length) {
  return m_file[part]->ha_index_next_same(buf, key, length);
}

/**
  Read next record when performing index scan backwards.

  Used to read backwards through the index (right to left, high to low).

  @param[in,out] buf  Read row in MySQL Row Format.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::index_prev_in_part(uint part, uchar *buf) {
  return m_file[part]->ha_index_prev(buf);
}

/**
  Start a read of one range with start and end key.

  @param part_id       Partition to start in.
  @param start_key     Specification of start key.
  @param end_key       Specification of end key.
  @param eq_range_arg  Is it equal range.
  @param sorted        Should records be returned in sorted order.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::read_range_first_in_part(uint part_id, uchar *buf,
                                             const key_range *start_key,
                                             const key_range *end_key,
                                             bool sorted) {
  int error;

  error = m_file[part_id]->read_range_first(start_key, end_key, get_eq_range(),
                                            sorted);
  if (!error && buf != nullptr) {
    memcpy(buf, table->record[0], m_rec_length);
  }
  return error;
}

/**
  Read next record in read of a range with start and end key in partition.

  @param part  Partition to read from.

  @return Operation status.
    @retval    0  Success
    @retval != 0  Error code
*/

int Partition_base::read_range_next_in_part(uint part, uchar *buf) {
  int error;
  error = m_file[part]->read_range_next();
  if (!error && buf != nullptr) {
    memcpy(buf, table->record[0], m_rec_length);
  }
  return error;
}

/*
  Whether the table or last access partition has TTL column
  Only used in replication error checking

  SYNOPSIS
    last_part_has_ttl_column

  RETURN VALUE
    true                    The table or last access partition contains TTL
                            column
    false                   Otherwise
*/
bool Partition_base::last_part_has_ttl_column() const {
  DBUG_ENTER("ha_partition::last_part_has_ttl_column");

  /*
    This is only used in replication idempotent error check to see if the
    last error may be caused by TTL - this means we should be able to rely
    on the saved last partition being where the error is happening and
    only check that partition.
   */
  handler *file = m_file[m_last_part];
  DBUG_RETURN(file->last_part_has_ttl_column());
}

/****************************************************************************
                MODULE information calls
****************************************************************************/

/*
  These are all first approximations of the extra, info, scan_time
  and read_time calls
*/

/**
  Helper function for sorting according to number of rows in descending order.
*/

int Partition_base::compare_number_of_records(Partition_base *me,
                                              const uint32 *a,
                                              const uint32 *b) {
  handler **file = me->m_file;
  /* Note: sorting in descending order! */
  if (file[*a]->stats.records > file[*b]->stats.records) return -1;
  if (file[*a]->stats.records < file[*b]->stats.records) return 1;
  return 0;
}

/*
  General method to gather info from handler

  SYNOPSIS
    info()
    flag              Specifies what info is requested

  RETURN VALUE
    NONE

  DESCRIPTION
    ::info() is used to return information to the optimizer.
    Currently this table handler doesn't implement most of the fields
    really needed. SHOW also makes use of this data
    Another note, if your handler doesn't proved exact record count,
    you will probably want to have the following in your code:
    if (records < 2)
      records = 2;
    The reason is that the server will optimize for cases of only a single
    record. If in a table scan you don't know the number of records
    it will probably be better to set records to two so you can return
    as many records as you need.

    Along with records a few more variables you may wish to set are:
      records
      deleted
      data_file_length
      index_file_length
      delete_length
      check_time
    Take a look at the public variables in handler.h for more information.

    Called in:
      filesort.cc
      ha_heap.cc
      item_sum.cc
      opt_sum.cc
      sql_delete.cc
     sql_delete.cc
     sql_derived.cc
      sql_select.cc
      sql_select.cc
      sql_select.cc
      sql_select.cc
      sql_select.cc
      sql_show.cc
      sql_show.cc
      sql_show.cc
      sql_show.cc
      sql_table.cc
      sql_union.cc
      sql_update.cc

    Some flags that are not implemented
      HA_STATUS_POS:
        This parameter is never used from the MySQL Server. It is checked in a
        place in MyISAM so could potentially be used by MyISAM specific
        programs.
      HA_STATUS_NO_LOCK:
      This is declared and often used. It's only used by MyISAM.
      It means that MySQL doesn't need the absolute latest statistics
      information. This may save the handler from doing internal locks while
      retrieving statistics data.
*/

int Partition_base::info(uint flag) {
  uint no_lock_flag = flag & HA_STATUS_NO_LOCK;
  uint extra_var_flag = flag & HA_STATUS_VARIABLE_EXTRA;
  int res, error = 0;
  DBUG_ENTER("Partition_base::info");

#ifndef DBUG_OFF
  if (bitmap_is_set_all(&(m_part_info->read_partitions)))
    DBUG_PRINT("info", ("All partitions are used"));
#endif /* DBUG_OFF */
  if (flag & HA_STATUS_AUTO) {
    DBUG_PRINT("info", ("HA_STATUS_AUTO"));
    if (!table->found_next_number_field) {
      stats.auto_increment_value = 0;
    } else {
      /* Must lock to avoid two concurrent initializations. */
      lock_auto_increment();
      if (part_share->auto_inc_initialized) {
        stats.auto_increment_value = part_share->next_auto_inc_val;
      } else {
        error = initialize_auto_increment(no_lock_flag != 0);
      }
      unlock_auto_increment();
    }
  }
  if (flag & HA_STATUS_VARIABLE) {
    uint i;
    DBUG_PRINT("info", ("HA_STATUS_VARIABLE"));
    /*
      Calculates statistical variables
      records:           Estimate of number records in table
      We report sum (always at least 2 if not empty)
      deleted:           Estimate of number holes in the table due to
      deletes
      We report sum
      data_file_length:  Length of data file, in principle bytes in table
      We report sum
      index_file_length: Length of index file, in principle bytes in
      indexes in the table
      We report sum
      delete_length: Length of free space easily used by new records in table
      We report sum
      mean_record_length:Mean record length in the table
      We calculate this
      check_time:        Time of last check (only applicable to MyISAM)
      We report last time of all underlying handlers
    */
    handler *file;
    stats.records = 0;
    stats.deleted = 0;
    stats.data_file_length = 0;
    stats.index_file_length = 0;
    stats.check_time = 0;
    stats.delete_length = 0;
    for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
         i = m_part_info->get_next_used_partition(i)) {
      file = m_file[i];
      res = file->info(HA_STATUS_VARIABLE | no_lock_flag | extra_var_flag);
      if (res && !error) {
        error = res;
      }
      stats.records += file->stats.records;
      stats.deleted += file->stats.deleted;
      stats.data_file_length += file->stats.data_file_length;
      stats.index_file_length += file->stats.index_file_length;
      stats.delete_length += file->stats.delete_length;
      if (file->stats.check_time > stats.check_time)
        stats.check_time = file->stats.check_time;
    }
    if (stats.records && stats.records < 2 &&
        !(m_file[0]->ha_table_flags() & HA_STATS_RECORDS_IS_EXACT))
      stats.records = 2;
    if (stats.records > 0)
      stats.mean_rec_length = (ulong)(stats.data_file_length / stats.records);
    else
      stats.mean_rec_length = 0;
  }
  if (flag & HA_STATUS_CONST) {
    DBUG_PRINT("info", ("HA_STATUS_CONST"));
    /*
      Recalculate loads of constant variables. MyISAM also sets things
      directly on the table share object.

      Check whether this should be fixed since handlers should not
      change things directly on the table object.

      Monty comment: This should NOT be changed!  It's the handlers
      responsibility to correct table->s->keys_xxxx information if keys
      have been disabled.

      The most important parameters set here is records per key on
      all indexes. block_size and primar key ref_length.

      For each index there is an array of rec_per_key.
      As an example if we have an index with three attributes a,b and c
      we will have an array of 3 rec_per_key.
      rec_per_key[0] is an estimate of number of records divided by
      number of unique values of the field a.
      rec_per_key[1] is an estimate of the number of records divided
      by the number of unique combinations of the fields a and b.
      rec_per_key[2] is an estimate of the number of records divided
      by the number of unique combinations of the fields a,b and c.

      Many handlers only set the value of rec_per_key when all fields
      are bound (rec_per_key[2] in the example above).

      If the handler doesn't support statistics, it should set all of the
      above to 0.

      We first scans through all partitions to get the one holding most rows.
      We will then allow the handler with the most rows to set
      the rec_per_key and use this as an estimate on the total table.

      max_data_file_length:     Maximum data file length
      We ignore it, is only used in
      SHOW TABLE STATUS
      max_index_file_length:    Maximum index file length
      We ignore it since it is never used
      block_size:               Block size used
      We set it to the value of the first handler
      ref_length:               We set this to the value calculated
      and stored in local object
      create_time:              Creation time of table

      So we calculate these constants by using the variables from the
      handler with most rows.
    */
    handler *file, **file_array;
    ulonglong max_records = 0;
    uint32 i = 0;
    uint32 handler_instance = 0;

    file_array = m_file;
    do {
      file = *file_array;
      /* Get variables if not already done */
      if (!(flag & HA_STATUS_VARIABLE) ||
          !m_part_info->is_partition_used(file_array - m_file)) {
        res = file->info(HA_STATUS_VARIABLE | no_lock_flag | extra_var_flag);
        if (res && !error) {
          error = res;
        }
      }
      if (file->stats.records > max_records) {
        max_records = file->stats.records;
        handler_instance = i;
      }
      i++;
    } while (*(++file_array));
    /*
      Sort the array of part_ids by number of records in
      in descending order.
    */
    varlen_sort(m_part_ids_sorted_by_num_of_records,
                m_part_ids_sorted_by_num_of_records + m_tot_parts,
                sizeof(uint32), [this](const uint32 *a, const uint32 *b) {
                  return m_file[*a]->stats.records < m_file[*b]->stats.records;
                });
    file = m_file[handler_instance];
    res = file->info(HA_STATUS_CONST | no_lock_flag);
    if (res && !error) {
      error = res;
    }
    stats.block_size = file->stats.block_size;
    stats.create_time = file->stats.create_time;
  }
  if (flag & HA_STATUS_ERRKEY) {
    handler *file = m_file[m_last_part];
    DBUG_PRINT("info", ("info: HA_STATUS_ERRKEY"));
    /*
      This flag is used to get index number of the unique index that
      reported duplicate key
      We will report the errkey on the last handler used and ignore the rest
      Note: all engines does not support HA_STATUS_ERRKEY, so set errkey.
    */
    file->errkey = errkey;
    res = file->info(HA_STATUS_ERRKEY | no_lock_flag);
    if (res && !error) {
      error = res;
    }
    errkey = file->errkey;
  }
  if (flag & HA_STATUS_TIME) {
    DBUG_PRINT("info", ("info: HA_STATUS_TIME"));
    /*
      This flag is used to set the latest update time of the table.
      Used by SHOW commands
      We will report the maximum of these times
    */
    stats.update_time = 0;
    for (uint i = m_part_info->get_first_used_partition(); i < m_tot_parts;
         i = m_part_info->get_next_used_partition(i)) {
      handler *file = m_file[i];
      res = file->info(HA_STATUS_TIME | no_lock_flag);
      if (res && !error) {
        error = res;
      }
      if (file->stats.update_time > stats.update_time)
        stats.update_time = file->stats.update_time;
    }
  }
  DBUG_RETURN(error);
}

void Partition_base::get_dynamic_partition_info(ha_statistics *stat_info,
                                                ha_checksum *check_sum,
                                                uint part_id) {
  handler *file = m_file[part_id];
  DBUG_ASSERT(bitmap_is_set(&(m_part_info->read_partitions), part_id));
  file->info(HA_STATUS_TIME | HA_STATUS_VARIABLE | HA_STATUS_VARIABLE_EXTRA |
             HA_STATUS_NO_LOCK);

  stat_info->records = file->stats.records;
  stat_info->mean_rec_length = file->stats.mean_rec_length;
  stat_info->data_file_length = file->stats.data_file_length;
  stat_info->max_data_file_length = file->stats.max_data_file_length;
  stat_info->index_file_length = file->stats.index_file_length;
  stat_info->delete_length = file->stats.delete_length;
  stat_info->create_time = static_cast<ulong>(file->stats.create_time);
  stat_info->update_time = file->stats.update_time;
  stat_info->check_time = file->stats.check_time;
  *check_sum = 0;
  if (file->ha_table_flags() & HA_HAS_CHECKSUM) *check_sum = file->checksum();
  return;
}

/**
  General function to prepare handler for certain behavior.

  @param[in]    operation       operation to execute

  @return       status
    @retval     0               success
    @retval     >0              error code

  @detail

  extra() is called whenever the server wishes to send a hint to
  the storage engine. The MyISAM engine implements the most hints.

  We divide the parameters into the following categories:
  1) Operations used by most handlers
  2) Operations used by some non-MyISAM handlers
  3) Operations used only by MyISAM
  4) Operations only used by temporary tables for query processing
  5) Operations only used by MyISAM internally
  6) Operations not used at all
  7) Operations only used by federated tables for query processing
  8) Operations only used by NDB
  9) Operations only used by MERGE
  10) Operations only used by InnoDB
  11) Operations only used by partitioning

  The partition handler need to handle category 1), 2), 3), 10) and 11).

  1) Operations used by most handlers
  -----------------------------------
  HA_EXTRA_RESET:
    This option is used by most handlers and it resets the handler state
    to the same state as after an open call. This includes releasing
    any READ CACHE or WRITE CACHE or other internal buffer used.

    It is called from the reset method in the handler interface. There are
    three instances where this is called.
    1) After completing a INSERT ... SELECT ... query the handler for the
       table inserted into is reset
    2) It is called from close_thread_table which in turn is called from
       close_thread_tables except in the case where the tables are locked
       in which case ha_commit_stmt is called instead.
       It is only called from here if refresh_version hasn't changed and the
       table is not an old table when calling close_thread_table.
       close_thread_tables is called from many places as a general clean up
       function after completing a query.
    3) It is called when deleting the QUICK_RANGE_SELECT object if the
       QUICK_RANGE_SELECT object had its own handler object. It is called
       immediatley before close of this local handler object.
  HA_EXTRA_KEYREAD:
  HA_EXTRA_NO_KEYREAD:
    These parameters are used to provide an optimisation hint to the handler.
    If HA_EXTRA_KEYREAD is set it is enough to read the index fields, for
    many handlers this means that the index-only scans can be used and it
    is not necessary to use the real records to satisfy this part of the
    query. Index-only scans is a very important optimisation for disk-based
    indexes. For main-memory indexes most indexes contain a reference to the
    record and thus KEYREAD only says that it is enough to read key fields.
    HA_EXTRA_NO_KEYREAD disables this for the handler, also HA_EXTRA_RESET
    will disable this option.
    The handler will set HA_KEYREAD_ONLY in its table flags to indicate this
    feature is supported.
  HA_EXTRA_FLUSH:
    Indication to flush tables to disk, is supposed to be used to
    ensure disk based tables are flushed at end of query execution.
    Currently is never used.
  HA_EXTRA_PREPARE_FOR_RENAME:
    Informs the handler we are about to attempt a rename of the table.
    For handlers that have share open files (MyISAM key-file and
    Archive writer) they must close the files before rename is possible
    on Windows. This handler will only forward this call, since during
    ALTER TABLE ... ADD/DROP/REORGANIZE/COALESCE/... PARTITION we will
    close and remove all instances before rename/drop and does not need
    special treatment for this flag.
  HA_EXTRA_FORCE_REOPEN:
    Only used by MyISAM and Archive, called when altering table,
    closing tables to enforce a reopen of the table files.
    This handler will only forward this call, since during
    ALTER TABLE ... ADD/DROP/REORGANIZE/COALESCE/... PARTITION we will
    close and remove all instances before rename/drop and does not need
    special treatment for this flag.

  2) Operations used by some non-MyISAM handlers
  ----------------------------------------------
  HA_EXTRA_KEYREAD_PRESERVE_FIELDS:
    This is a strictly InnoDB feature that is more or less undocumented.
    When it is activated InnoDB copies field by field from its fetch
    cache instead of all fields in one memcpy. Have no idea what the
    purpose of this is.
    Cut from include/my_base.h:
    When using HA_EXTRA_KEYREAD, overwrite only key member fields and keep
    other fields intact. When this is off (by default) InnoDB will use memcpy
    to overwrite entire row.
  HA_EXTRA_IGNORE_DUP_KEY:
  HA_EXTRA_NO_IGNORE_DUP_KEY:
    Informs the handler to we will not stop the transaction if we get an
    duplicate key errors during insert/upate.
    Always called in pair, triggered by INSERT IGNORE and other similar
    SQL constructs.
    Not used by MyISAM.

  3) Operations used only by MyISAM
  ---------------------------------
  HA_EXTRA_NORMAL:
    Only used in MyISAM to reset quick mode, not implemented by any other
    handler. Quick mode is also reset in MyISAM by HA_EXTRA_RESET.

    It is called after completing a successful DELETE query if the QUICK
    option is set.

  HA_EXTRA_QUICK:
    When the user does DELETE QUICK FROM table where-clause; this extra
    option is called before the delete query is performed and
    HA_EXTRA_NORMAL is called after the delete query is completed.
    Temporary tables used internally in MySQL always set this option

    The meaning of quick mode is that when deleting in a B-tree no merging
    of leafs is performed. This is a common method and many large DBMS's
    actually only support this quick mode since it is very difficult to
    merge leaves in a tree used by many threads concurrently.

  HA_EXTRA_CACHE:
    This flag is usually set with extra_opt along with a cache size.
    The size of this buffer is set by the user variable
    record_buffer_size. The value of this cache size is the amount of
    data read from disk in each fetch when performing a table scan.
    This means that before scanning a table it is normal to call
    extra with HA_EXTRA_CACHE and when the scan is completed to call
    HA_EXTRA_NO_CACHE to release the cache memory.

    Some special care is taken when using this extra parameter since there
    could be a write ongoing on the table in the same statement. In this
    one has to take special care since there might be a WRITE CACHE as
    well. HA_EXTRA_CACHE specifies using a READ CACHE and using
    READ CACHE and WRITE CACHE at the same time is not possible.

    Only MyISAM currently use this option.

    It is set when doing full table scans using rr_sequential and
    reset when completing such a scan with end_read_record
    (resetting means calling extra with HA_EXTRA_NO_CACHE).

    It is set in filesort.cc for MyISAM internal tables and it is set in
    a multi-update where HA_EXTRA_CACHE is called on a temporary result
    table and after that ha_rnd_init(0) on table to be updated
    and immediately after that HA_EXTRA_NO_CACHE on table to be updated.

    Apart from that it is always used from init_read_record but not when
    used from UPDATE statements. It is not used from DELETE statements
    with ORDER BY and LIMIT but it is used in normal scan loop in DELETE
    statements. The reason here is that DELETE's in MyISAM doesn't move
    existings data rows.

    It is also set in copy_data_between_tables when scanning the old table
    to copy over to the new table.
    And it is set in join_init_read_record where quick objects are used
    to perform a scan on the table. In this case the full table scan can
    even be performed multiple times as part of the nested loop join.

    For purposes of the partition handler it is obviously necessary to have
    special treatment of this extra call. If we would simply pass this
    extra call down to each handler we would allocate
    cache size * no of partitions amount of memory and this is not
    necessary since we will only scan one partition at a time when doing
    full table scans.

    Thus we treat it by first checking whether we have MyISAM handlers in
    the table, if not we simply ignore the call and if we have we will
    record the call but will not call any underlying handler yet. Then
    when performing the sequential scan we will check this recorded value
    and call extra_opt whenever we start scanning a new partition.

  HA_EXTRA_NO_CACHE:
    When performing a UNION SELECT HA_EXTRA_NO_CACHE is called from the
    flush method in the Query_result_union class.
    See HA_EXTRA_RESET_STATE for use in conjunction with delete_all_rows().

    It should be ok to call HA_EXTRA_NO_CACHE on all underlying handlers
    if they are MyISAM handlers. Other handlers we can ignore the call
    for. If no cache is in use they will quickly return after finding
    this out. And we also ensure that all caches are disabled and no one
    is left by mistake.
    In the future this call will probably be deleted and we will instead call
    ::reset();

  HA_EXTRA_WRITE_CACHE:
    See above, called from various places. It is mostly used when we
    do INSERT ... SELECT
    No special handling to save cache space is developed currently.

  HA_EXTRA_PREPARE_FOR_UPDATE:
    This is called as part of a multi-table update. When the table to be
    updated is also scanned then this informs MyISAM handler to drop any
    caches if dynamic records are used (fixed size records do not care
    about this call). We pass this along to the first partition to scan, and
    flag that it is to be called after HA_EXTRA_CACHE when moving to the next
    partition to scan.

  HA_EXTRA_PREPARE_FOR_DROP:
    Only used by MyISAM, called in preparation for a DROP TABLE.
    It's used mostly by Windows that cannot handle dropping an open file.
    On other platforms it has the same effect as HA_EXTRA_FORCE_REOPEN.

  HA_EXTRA_READCHECK:
  HA_EXTRA_NO_READCHECK:
    Only one call to HA_EXTRA_NO_READCHECK from ha_open where it says that
    this is not needed in SQL. The reason for this call is that MyISAM sets
    the READ_CHECK_USED in the open call so the call is needed for MyISAM
    to reset this feature.
    The idea with this parameter was to inform of doing/not doing a read
    check before applying an update. Since SQL always performs a read before
    applying the update No Read Check is needed in MyISAM as well.

    This is a cut from Docs/myisam.txt
     Sometimes you might want to force an update without checking whether
     another user has changed the record since you last read it. This is
     somewhat dangerous, so it should ideally not be used. That can be
     accomplished by wrapping the mi_update() call in two calls to mi_extra(),
     using these functions:
     HA_EXTRA_NO_READCHECK=5                 No readcheck on update
     HA_EXTRA_READCHECK=6                    Use readcheck (def)


  4) Operations only used by temporary tables for query processing
  ----------------------------------------------------------------
  HA_EXTRA_RESET_STATE:
    Same as reset() except that buffers are not released. If there is
    a READ CACHE it is reinit'ed. A cache is reinit'ed to restart reading
    or to change type of cache between READ CACHE and WRITE CACHE.

    This extra function is always called immediately before calling
    delete_all_rows on the handler for temporary tables.
    There are cases however when HA_EXTRA_RESET_STATE isn't called in
    a similar case for a temporary table in sql_union.cc and in two other
    cases HA_EXTRA_NO_CACHE is called before and HA_EXTRA_WRITE_CACHE
    called afterwards.
    The case with HA_EXTRA_NO_CACHE and HA_EXTRA_WRITE_CACHE means
    disable caching, delete all rows and enable WRITE CACHE. This is
    used for temporary tables containing distinct sums and a
    functional group.

    The only case that delete_all_rows is called on non-temporary tables
    is in sql_delete.cc when DELETE FROM table; is called by a user.
    In this case no special extra calls are performed before or after this
    call.

    The partition handler should not need to bother about this one. It
    should never be called.

  HA_EXTRA_NO_ROWS:
    Don't insert rows indication to HEAP and MyISAM, only used by temporary
    tables used in query processing.
    Not handled by partition handler.

  5) Operations only used by MyISAM internally
  --------------------------------------------
  HA_EXTRA_REINIT_CACHE:
    This call reinitializes the READ CACHE described above if there is one
    and otherwise the call is ignored.

    We can thus safely call it on all underlying handlers if they are
    MyISAM handlers. It is however never called so we don't handle it at all.
  HA_EXTRA_FLUSH_CACHE:
    Flush WRITE CACHE in MyISAM. It is only from one place in the code.
    This is in sql_insert.cc where it is called if the table_flags doesn't
    contain HA_DUPLICATE_POS. The only handler having the HA_DUPLICATE_POS
    set is the MyISAM handler and so the only handler not receiving this
    call is MyISAM.
    Thus in effect this call is called but never used. Could be removed
    from sql_insert.cc
  HA_EXTRA_NO_USER_CHANGE:
    Only used by MyISAM, never called.
    Simulates lock_type as locked.
  HA_EXTRA_WAIT_LOCK:
  HA_EXTRA_WAIT_NOLOCK:
    Only used by MyISAM, called from MyISAM handler but never from server
    code on top of the handler.
    Sets lock_wait on/off
  HA_EXTRA_NO_KEYS:
    Only used MyISAM, only used internally in MyISAM handler, never called
    from server level.
  HA_EXTRA_KEYREAD_CHANGE_POS:
  HA_EXTRA_REMEMBER_POS:
  HA_EXTRA_RESTORE_POS:
  HA_EXTRA_PRELOAD_BUFFER_SIZE:
  HA_EXTRA_CHANGE_KEY_TO_DUP:
  HA_EXTRA_CHANGE_KEY_TO_UNIQUE:
    Only used by MyISAM, never called.

  6) Operations not used at all
  -----------------------------
  HA_EXTRA_KEY_CACHE:
  HA_EXTRA_NO_KEY_CACHE:
    This parameters are no longer used and could be removed.

  7) Operations only used by federated tables for query processing
  ----------------------------------------------------------------
  HA_EXTRA_INSERT_WITH_UPDATE:
    Inform handler that an "INSERT...ON DUPLICATE KEY UPDATE" will be
    executed. This condition is unset by HA_EXTRA_NO_IGNORE_DUP_KEY.

  8) Operations only used by NDB
  ------------------------------
  HA_EXTRA_DELETE_CANNOT_BATCH:
  HA_EXTRA_UPDATE_CANNOT_BATCH:
    Inform handler that delete_row()/update_row() cannot batch deletes/updates
    and should perform them immediately. This may be needed when table has
    AFTER DELETE/UPDATE triggers which access to subject table.
    These flags are reset by the handler::extra(HA_EXTRA_RESET) call.

  9) Operations only used by MERGE
  ------------------------------
  HA_EXTRA_ADD_CHILDREN_LIST:
  HA_EXTRA_ATTACH_CHILDREN:
  HA_EXTRA_IS_ATTACHED_CHILDREN:
  HA_EXTRA_DETACH_CHILDREN:
    Special actions for MERGE tables. Ignore.

  10) Operations only used by InnoDB
  ----------------------------------
  HA_EXTRA_EXPORT:
    Prepare table for export
    (e.g. quiesce the table and write table metadata).

  11) Operations only used by partitioning
  ------------------------------
  HA_EXTRA_SECONDARY_SORT_ROWID:
    INDEX_MERGE type of execution, needs to do secondary sort by
    ROWID (handler::ref).
*/

int Partition_base::extra(enum ha_extra_function operation) {
  DBUG_ENTER("Partition_base:extra");
  DBUG_PRINT("info", ("operation: %d", (int)operation));

  switch (operation) {
      /* Category 1), used by most handlers */
    case HA_EXTRA_KEYREAD:
    case HA_EXTRA_NO_KEYREAD:
    case HA_EXTRA_FLUSH:
    case HA_EXTRA_PREPARE_FOR_RENAME:
    case HA_EXTRA_FORCE_REOPEN:
      DBUG_RETURN(loop_extra(operation));
      break;

      /* Category 2), used by non-MyISAM handlers */
    case HA_EXTRA_IGNORE_DUP_KEY:
    case HA_EXTRA_NO_IGNORE_DUP_KEY:
    case HA_EXTRA_KEYREAD_PRESERVE_FIELDS: {
      DBUG_RETURN(loop_extra(operation));
      break;
    }

    /* Category 3), used by MyISAM handlers */
    case HA_EXTRA_PREPARE_FOR_UPDATE:
    case HA_EXTRA_NORMAL:
    case HA_EXTRA_QUICK:
    case HA_EXTRA_PREPARE_FOR_DROP:
    case HA_EXTRA_NO_READCHECK: {
      break;
    }
    case HA_EXTRA_IGNORE_NO_KEY:
    case HA_EXTRA_NO_IGNORE_NO_KEY: {
      /*
        Ignore as these are specific to NDB for handling
        idempotency
       */
      break;
    }
    case HA_EXTRA_WRITE_CAN_REPLACE:
    case HA_EXTRA_WRITE_CANNOT_REPLACE: {
      /*
        Informs handler that write_row() can replace rows which conflict
        with row being inserted by PK/unique key without reporting error
        to the SQL-layer.

        This optimization is not safe for partitioned table in general case
        since we may have to put new version of row into partition which is
        different from partition in which old version resides (for example
        when we partition by non-PK column or by some column which is not
        part of unique key which were violated).
        And since NDB which is the only engine at the moment that supports
        this optimization handles partitioning on its own we simple disable
        it here. (BTW for NDB this optimization is safe since it supports
        only KEY partitioning and won't use this optimization for tables
        which have additional unique constraints).
      */
      break;
    }
      /* Category 7), used by federated handlers */
    case HA_EXTRA_INSERT_WITH_UPDATE:
      DBUG_RETURN(loop_extra(operation));
      /* Category 8) Operations only used by NDB */
    case HA_EXTRA_DELETE_CANNOT_BATCH:
    case HA_EXTRA_UPDATE_CANNOT_BATCH: {
      /* Currently only NDB use the *_CANNOT_BATCH */
      break;
    }
      /* Category 9) Operations only used by MERGE */
    case HA_EXTRA_ADD_CHILDREN_LIST:
    case HA_EXTRA_ATTACH_CHILDREN:
    case HA_EXTRA_IS_ATTACHED_CHILDREN:
    case HA_EXTRA_DETACH_CHILDREN: {
      /* Special actions for MERGE tables. Ignore. */
      break;
    }
    /*
      http://dev.mysql.com/doc/refman/5.1/en/partitioning-limitations.html
      says we no longer support logging to partitioned tables, so we fail
      here.
    */
    case HA_EXTRA_MARK_AS_LOG_TABLE:
      DBUG_RETURN(ER_UNSUPORTED_LOG_ENGINE);
      /* Category 10), used by InnoDB handlers */
    case HA_EXTRA_BEGIN_ALTER_COPY:
    case HA_EXTRA_END_ALTER_COPY:
    case HA_EXTRA_EXPORT:
      DBUG_RETURN(loop_extra(operation));
      /* Category 11) Operations only used by partitioning. */
    case HA_EXTRA_SECONDARY_SORT_ROWID: {
      // TODO: Remove this and add a flag to index_init instead,
      // so we can avoid allocating ref_length bytes for every used partition
      // in init_record_priority_queue()!
      /* index_init(sorted=true) must have been called! */
      DBUG_ASSERT(m_ordered);
      DBUG_ASSERT(m_ordered_rec_buffer);
      /* No index_read call must have been done! */
      DBUG_ASSERT(m_queue->empty());
      /* If not PK is set as secondary sort, do secondary sort by rowid/ref. */
      if (!m_curr_key_info[1]) {
        m_ref_usage = Partition_helper::REF_USED_FOR_SORT;
        m_queue->m_fun = key_and_ref_cmp;
      }
      break;
    }
    default: {
      /* Temporary crash to discover what is wrong */
      DBUG_ASSERT(0);
      break;
    }
  }
  DBUG_RETURN(0);
}

/**
  Special extra call to reset extra parameters

  @return Operation status.
    @retval >0 Error code
    @retval 0  Success

  @note Called at end of each statement to reset buffers.
  To avoid excessive calls, the m_partitions_to_reset bitmap keep records
  of which partitions that have been used in extra(), external_lock() or
  start_stmt() and is needed to be called.
*/

int Partition_base::reset(void) {
  int result = 0;
  int tmp;
  uint i;
  DBUG_ENTER("Partition_base::reset");

  for (i = bitmap_get_first_set(&m_partitions_to_reset); i < m_tot_parts;
       i = bitmap_get_next_set(&m_partitions_to_reset, i)) {
    if ((tmp = m_file[i]->ha_reset())) result = tmp;
  }
  bitmap_clear_all(&m_partitions_to_reset);
  DBUG_RETURN(result);
}

/*
  Call extra on all partitions

  SYNOPSIS
    loop_extra()
    operation             extra operation type

  RETURN VALUE
    >0                    Error code
    0                     Success
*/

int Partition_base::loop_extra(enum ha_extra_function operation) {
  int result = 0, tmp;
  uint i;
  DBUG_ENTER("Partition_base::loop_extra()");

  for (i = bitmap_get_first_set(&m_part_info->lock_partitions); i < m_tot_parts;
       i = bitmap_get_next_set(&m_part_info->lock_partitions, i)) {
    if ((tmp = m_file[i]->extra(operation))) result = tmp;
  }
  /* Add all used partitions to be called in reset(). */
  bitmap_union(&m_partitions_to_reset, &m_part_info->lock_partitions);
  DBUG_RETURN(result);
}

/****************************************************************************
                MODULE optimiser support
****************************************************************************/

/**
  Minimum number of rows to base optimizer estimate on.
*/

ha_rows Partition_base::min_rows_for_estimate() {
  uint i, max_used_partitions, tot_used_partitions;
  DBUG_ENTER("Partition_base::min_rows_for_estimate");

  tot_used_partitions = m_part_info->num_partitions_used();

  /*
    All partitions might have been left as unused during partition pruning
    due to, for example, an impossible WHERE condition. Nonetheless, the
    optimizer might still attempt to perform (e.g. range) analysis where an
    estimate of the the number of rows is calculated using records_in_range.
    Hence, to handle this and other possible cases, use zero as the minimum
    number of rows to base the estimate on if no partition is being used.
  */
  if (!tot_used_partitions) DBUG_RETURN(0);

  /*
    Allow O(log2(tot_partitions)) increase in number of used partitions.
    This gives O(tot_rows/log2(tot_partitions)) rows to base the estimate on.
    I.e when the total number of partitions doubles, allow one more
    partition to be checked.
  */
  i = 2;
  max_used_partitions = 1;
  while (i < m_tot_parts) {
    max_used_partitions++;
    i = i << 1;
  }
  if (max_used_partitions > tot_used_partitions)
    max_used_partitions = tot_used_partitions;

  /* stats.records is already updated by the info(HA_STATUS_VARIABLE) call. */
  DBUG_PRINT("info", ("max_used_partitions: %u tot_rows: %lu",
                      max_used_partitions, (ulong)stats.records));
  DBUG_PRINT(
      "info",
      ("tot_used_partitions: %u min_rows_to_check: %lu", tot_used_partitions,
       (ulong)stats.records * max_used_partitions / tot_used_partitions));
  DBUG_RETURN(stats.records * max_used_partitions / tot_used_partitions);
}

/**
  Get the biggest used partition.

  Starting at the N:th biggest partition and skips all non used
  partitions, returning the biggest used partition found

  @param[in,out] part_index  Skip the *part_index biggest partitions

  @return The biggest used partition with index not lower than *part_index.
    @retval NO_CURRENT_PART_ID     No more partition used.
    @retval != NO_CURRENT_PART_ID  partition id of biggest used partition with
                                   index >= *part_index supplied. Note that
                                   *part_index will be updated to the next
                                   partition index to use.
*/

uint Partition_base::get_biggest_used_partition(uint *part_index) {
  uint part_id;
  while ((*part_index) < m_tot_parts) {
    part_id = m_part_ids_sorted_by_num_of_records[(*part_index)++];
    if (m_part_info->is_partition_used(part_id)) return part_id;
  }
  return NO_CURRENT_PART_ID;
}

/*
  Return time for a scan of the table

  SYNOPSIS
    scan_time()

  RETURN VALUE
    time for scan
*/

double Partition_base::scan_time() {
  double scan_time = 0;
  uint i;
  DBUG_ENTER("Partition_base::scan_time");

  for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
       i = m_part_info->get_next_used_partition(i))
    scan_time += m_file[i]->scan_time();
  DBUG_RETURN(scan_time);
}

/**
  Find number of records in a range.
  @param inx      Index number
  @param min_key  Start of range
  @param max_key  End of range

  @return Number of rows in range.

  Given a starting key, and an ending key estimate the number of rows that
  will exist between the two. max_key may be empty which in case determine
  if start_key matches any rows.
*/

ha_rows Partition_base::records_in_range(uint inx, key_range *min_key,
                                         key_range *max_key) {
  ha_rows min_rows_to_check, rows, estimated_rows = 0, checked_rows = 0;
  uint partition_index = 0, part_id;
  DBUG_ENTER("Partition_base::records_in_range");

  min_rows_to_check = min_rows_for_estimate();

  while ((part_id = get_biggest_used_partition(&partition_index)) !=
         NO_CURRENT_PART_ID) {
    rows = m_file[part_id]->records_in_range(inx, min_key, max_key);

    DBUG_PRINT("info", ("part %u match %lu rows of %lu", part_id, (ulong)rows,
                        (ulong)m_file[part_id]->stats.records));

    if (rows == HA_POS_ERROR) DBUG_RETURN(HA_POS_ERROR);
    estimated_rows += rows;
    checked_rows += m_file[part_id]->stats.records;
    /*
      Returning 0 means no rows can be found, so we must continue
      this loop as long as we have estimated_rows == 0.
      Also many engines return 1 to indicate that there may exist
      a matching row, we do not normalize this by dividing by number of
      used partitions, but leave it to be returned as a sum, which will
      reflect that we will need to scan each partition's index.

      Note that this statistics may not always be correct, so we must
      continue even if the current partition has 0 rows, since we might have
      deleted rows from the current partition, or inserted to the next
      partition.
    */
    if (estimated_rows && checked_rows && checked_rows >= min_rows_to_check) {
      DBUG_PRINT(
          "info",
          ("records_in_range(inx %u): %lu (%lu * %lu / %lu)", inx,
           (ulong)(estimated_rows * stats.records / checked_rows),
           (ulong)estimated_rows, (ulong)stats.records, (ulong)checked_rows));
      DBUG_RETURN(estimated_rows * stats.records / checked_rows);
    }
  }
  DBUG_PRINT("info",
             ("records_in_range(inx %u): %lu", inx, (ulong)estimated_rows));
  DBUG_RETURN(estimated_rows);
}

/**
  Estimate upper bound of number of rows.

  @return Number of rows.
*/

ha_rows Partition_base::estimate_rows_upper_bound() {
  ha_rows rows, tot_rows = 0;
  handler **file = m_file;
  DBUG_ENTER("Partition_base::estimate_rows_upper_bound");

  do {
    if (m_part_info->is_partition_used(file - m_file)) {
      rows = (*file)->estimate_rows_upper_bound();
      if (rows == HA_POS_ERROR) DBUG_RETURN(HA_POS_ERROR);
      tot_rows += rows;
    }
  } while (*(++file));
  DBUG_RETURN(tot_rows);
}

/*
  Get time to read

  SYNOPSIS
    read_time()
    index                Index number used
    ranges               Number of ranges
    rows                 Number of rows

  RETURN VALUE
    time for read

  DESCRIPTION
    This will be optimised later to include whether or not the index can
    be used with partitioning. To achieve we need to add another parameter
    that specifies how many of the index fields that are bound in the ranges.
    Possibly added as a new call to handlers.
*/

double Partition_base::read_time(uint index, uint ranges, ha_rows rows) {
  DBUG_ENTER("Partition_base::read_time");

  DBUG_RETURN(m_file[0]->read_time(index, ranges, rows));
}

/**
  Number of rows in table. see handler.h
  @param[out] num_rows Number of records in the table (after pruning!)
  @return possible error code.
*/

int Partition_base::records(ha_rows *num_rows) {
  ha_rows tot_rows = 0;
  uint i;
  DBUG_ENTER("Partition_base::records");

  for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
       i = m_part_info->get_next_used_partition(i)) {
    int error = m_file[i]->ha_records(num_rows);
    if (error != 0) DBUG_RETURN(error);
    tot_rows += *num_rows;
  }
  *num_rows = tot_rows;
  DBUG_RETURN(0);
}

/****************************************************************************
                MODULE print messages
****************************************************************************/

bool Partition_base::continue_partition_copying_on_error(int error) {
  THD *thd = get_thd();
  if ((error == HA_ERR_NO_PARTITION_FOUND) &&
      (thd->lex->alter_info != nullptr) &&
      (thd->lex->alter_info->flags & (Alter_info::ALTER_REORGANIZE_PARTITION |
                                      Alter_info::ALTER_DROP_PARTITION)))
    return true;
  return false;
}

void Partition_base::print_error(int error, myf errflag) {
  DBUG_ENTER("Partition_base::print_error");
  if (print_partition_error(error)) {
    /* Not a partitioning error. */
    /* In case m_file has not been initialized, like in bug#42438 */
    if (m_file) {
      if (m_last_part >= m_tot_parts) {
        m_last_part = 0;
      }
      m_file[m_last_part]->print_error(error, errflag);
    } else
      handler::print_error(error, errflag);
  }
  DBUG_VOID_RETURN;
}

bool Partition_base::get_error_message(int error, String *buf) {
  DBUG_ENTER("Partition_base::get_error_message");

  /* Should probably look for my own errors first */

  /* In case m_file has not been initialized, like in bug#42438 */
  if (m_file) DBUG_RETURN(m_file[m_last_part]->get_error_message(error, buf));
  DBUG_RETURN(handler::get_error_message(error, buf));
}

/****************************************************************************
                MODULE in-place ALTER
****************************************************************************/
/**
  Get table flags.
*/

handler::Table_flags Partition_base::table_flags() const {
  uint first_used_partition = 0;
  DBUG_ENTER("Partition_base::table_flags");
  /*
    if (m_handler_status < handler_initialized ||
        m_handler_status >= handler_closed)
      DBUG_RETURN(PARTITION_ENABLED_TABLE_FLAGS);
  */
  if (!m_file) {
    std::unique_ptr<handler, Destroy_only<handler>> file(
        get_file_handler(nullptr, ha_thd()->mem_root));
    DBUG_RETURN((file->ha_table_flags() & ~(PARTITION_DISABLED_TABLE_FLAGS)) |
                (PARTITION_ENABLED_TABLE_FLAGS));
  }

  if (get_lock_type() != F_UNLCK) {
    /*
      The flags are cached after external_lock, and may depend on isolation
      level. So we should use a locked partition to get the correct flags.
    */
    first_used_partition = bitmap_get_first_set(&m_part_info->lock_partitions);
    if (first_used_partition == MY_BIT_NONE) first_used_partition = 0;
  }
  DBUG_RETURN((m_file[first_used_partition]->ha_table_flags() &
               ~(PARTITION_DISABLED_TABLE_FLAGS)) |
              (PARTITION_ENABLED_TABLE_FLAGS));
}

/**
  check if copy of data is needed in alter table.
*/
bool Partition_base::check_if_incompatible_data(HA_CREATE_INFO *create_info,
                                                uint table_changes) {
  handler **file;
  bool ret = COMPATIBLE_DATA_YES;

  /*
    The check for any partitioning related changes have already been done
    in mysql_alter_table (by fix_partition_func), so it is only up to
    the underlying handlers.
  */
  for (file = m_file; *file; file++)
    if ((ret = (*file)->check_if_incompatible_data(
             create_info, table_changes)) != COMPATIBLE_DATA_YES)
      break;
  return ret;
}

/**
  Support of in-place alter table.
*/

/**
  Helper class for in-place alter, see handler.h
*/

class Partition_base_inplace_ctx : public inplace_alter_handler_ctx {
 public:
  inplace_alter_handler_ctx **handler_ctx_array;

 private:
  uint m_tot_parts;

 public:
  Partition_base_inplace_ctx(THD * /*thd*/, uint tot_parts)
      : inplace_alter_handler_ctx(),
        handler_ctx_array(nullptr),
        m_tot_parts(tot_parts) {}

  ~Partition_base_inplace_ctx() override {
    if (handler_ctx_array) {
      for (uint index = 0; index < m_tot_parts; index++)
        destroy(handler_ctx_array[index]);
    }
  }
};

enum_alter_inplace_result Partition_base::check_if_supported_inplace_alter(
    TABLE *altered_table, Alter_inplace_info *ha_alter_info) {
  uint index = 0;
  enum_alter_inplace_result result = HA_ALTER_INPLACE_NO_LOCK;
  Partition_base_inplace_ctx *part_inplace_ctx;
  bool first_is_set = false;
  THD *thd = ha_thd();

  DBUG_ENTER("Partition_base::check_if_supported_inplace_alter");
  /*
    Support inplace change of KEY () -> KEY ALGORITHM = N ()
    and UPGRADE PARTITIONING.
    Any other change would set partition_changed in
    prep_alter_part_table() in mysql_alter_table().
  */
  if (ha_alter_info->alter_info->flags == Alter_info::ALTER_PARTITION)
    DBUG_RETURN(HA_ALTER_INPLACE_NO_LOCK);

  /* Fail DROP partitions because partitions cannot be truncated */
  if (!opt_parthandler_allow_drop_partition &&
      ha_alter_info->alter_info->flags & Alter_info::ALTER_DROP_PARTITION) {
    my_error(ER_ALLOW_DROP_PARTITION_PREVENTED, MYF(0));
    DBUG_RETURN(HA_ALTER_ERROR);
  }

  if (ha_alter_info->alter_info->flags &
      (Alter_info::ALTER_COALESCE_PARTITION |
       Alter_info::ALTER_REORGANIZE_PARTITION |
       Alter_info::ALTER_EXCHANGE_PARTITION)) {
    push_warning_printf(thd, Sql_condition::SL_WARNING, HA_ERR_UNSUPPORTED,
                        "Inplace partition altering is not supported");
    DBUG_RETURN(HA_ALTER_INPLACE_NOT_SUPPORTED);
  }
  if ((ha_alter_info->alter_info->flags & Alter_info::ALTER_ADD_PARTITION) &&
      (ha_alter_info->modified_part_info->part_type == partition_type::HASH)) {
    push_warning_printf(thd, Sql_condition::SL_WARNING, HA_ERR_UNSUPPORTED,
                        "Inplace partition altering is not supported");
    DBUG_RETURN(HA_ALTER_INPLACE_NOT_SUPPORTED);
  }
  /* We cannot allow INPLACE to change order of KEY partitioning fields! */
  if (ha_alter_info->handler_flags &
      Alter_inplace_info::ALTER_STORED_COLUMN_ORDER) {
    if (!m_part_info->same_key_column_order(
            &ha_alter_info->alter_info->create_list)) {
      DBUG_RETURN(HA_ALTER_INPLACE_NOT_SUPPORTED);
    }
  }

  part_inplace_ctx =
      new (thd->mem_root) Partition_base_inplace_ctx(thd, m_tot_parts);
  if (!part_inplace_ctx) DBUG_RETURN(HA_ALTER_ERROR);

  part_inplace_ctx->handler_ctx_array =
      (inplace_alter_handler_ctx **)thd->alloc(
          sizeof(inplace_alter_handler_ctx *) * (m_tot_parts + 1));
  if (!part_inplace_ctx->handler_ctx_array) DBUG_RETURN(HA_ALTER_ERROR);

  /* Set all to nullptr, including the terminating one. */
  for (index = 0; index <= m_tot_parts; index++)
    part_inplace_ctx->handler_ctx_array[index] = nullptr;

  for (index = 0; index < m_tot_parts; index++) {
    enum_alter_inplace_result p_result =
        m_file[index]->check_if_supported_inplace_alter(altered_table,
                                                        ha_alter_info);
    part_inplace_ctx->handler_ctx_array[index] = ha_alter_info->handler_ctx;

    if (index == 0) {
      first_is_set = (ha_alter_info->handler_ctx != nullptr);
    } else if (first_is_set != (ha_alter_info->handler_ctx != nullptr)) {
      /* Either none or all partitions must set handler_ctx! */
      DBUG_ASSERT(0);
      DBUG_RETURN(HA_ALTER_ERROR);
    }
    if (p_result < result) result = p_result;
    if (result == HA_ALTER_ERROR) break;
  }

  ha_alter_info->handler_ctx = part_inplace_ctx;
  /*
    To indicate for future inplace calls that there are several
    partitions/handlers that need to be committed together,
    we set group_commit_ctx to the nullptr terminated array of
    the partitions handlers.
  */
  ha_alter_info->group_commit_ctx = part_inplace_ctx->handler_ctx_array;

  if (ha_alter_info->alter_info->flags & Alter_info::ALTER_DROP_PARTITION) {
    DBUG_RETURN(HA_ALTER_INPLACE_NO_LOCK_AFTER_PREPARE);
  }

  if (ha_alter_info->handler_flags & Alter_inplace_info::ADD_PARTITION) {
    switch (ha_alter_info->modified_part_info->part_type) {
      case partition_type::RANGE:
      case partition_type::LIST:
        DBUG_RETURN(HA_ALTER_INPLACE_NO_LOCK_AFTER_PREPARE);
      default:
        DBUG_RETURN(HA_ALTER_INPLACE_SHARED_LOCK_AFTER_PREPARE);
    }
  }
  DBUG_RETURN(result);
}

bool Partition_base::prepare_inplace_alter_table(
    TABLE *altered_table, Alter_inplace_info *ha_alter_info,
    const dd::Table *old_table_def, dd::Table *new_table_def) {
  uint index = 0;
  bool error = false;
  Partition_base_inplace_ctx *part_inplace_ctx;

  DBUG_ENTER("Partition_base::prepare_inplace_alter_table");

  /*
    Changing to similar partitioning, only update metadata.
    Non allowed changes would be catched in prep_alter_part_table().
  */
  if (ha_alter_info->alter_info->flags == Alter_info::ALTER_PARTITION)
    DBUG_RETURN(false);

  part_inplace_ctx = static_cast<class Partition_base_inplace_ctx *>(
      ha_alter_info->handler_ctx);

  for (index = 0; index < m_tot_parts && !error; index++) {
    ha_alter_info->handler_ctx = part_inplace_ctx->handler_ctx_array[index];
    if (m_file[index]->ha_prepare_inplace_alter_table(
            altered_table, ha_alter_info, old_table_def, new_table_def))
      error = true;
    part_inplace_ctx->handler_ctx_array[index] = ha_alter_info->handler_ctx;
  }
  ha_alter_info->handler_ctx = part_inplace_ctx;

  DBUG_RETURN(error);
}

bool Partition_base::inplace_alter_table(TABLE *altered_table,
                                         Alter_inplace_info *ha_alter_info,
                                         const dd::Table *old_table_def,
                                         dd::Table *new_table_def) {
  uint index = 0;
  bool error = false;
  Partition_base_inplace_ctx *part_inplace_ctx;

  DBUG_ENTER("Partition_base::inplace_alter_table");

  /*
    Changing to similar partitioning, only update metadata.
    Non allowed changes would be catched in prep_alter_part_table().
  */
  if (ha_alter_info->alter_info->flags == Alter_info::ALTER_PARTITION)
    DBUG_RETURN(false);

  part_inplace_ctx = static_cast<class Partition_base_inplace_ctx *>(
      ha_alter_info->handler_ctx);

  for (index = 0; index < m_tot_parts && !error; index++) {
    ha_alter_info->handler_ctx = part_inplace_ctx->handler_ctx_array[index];
    if (m_file[index]->ha_inplace_alter_table(altered_table, ha_alter_info,
                                              old_table_def, new_table_def))
      error = true;
    part_inplace_ctx->handler_ctx_array[index] = ha_alter_info->handler_ctx;
  }
  ha_alter_info->handler_ctx = part_inplace_ctx;

  DBUG_RETURN(error);
}

/*
  Note that this function will try rollback failed ADD INDEX by
  executing DROP INDEX for the indexes that were committed (if any)
  before the error occured. This means that the underlying storage
  engine must be able to drop index in-place with X-lock held.
  (As X-lock will be held here if new indexes are to be committed)
*/

bool Partition_base::commit_inplace_alter_table(
    TABLE *altered_table, Alter_inplace_info *ha_alter_info, bool commit,
    const dd::Table *old_table_def, dd::Table *new_table_def) {
  Partition_base_inplace_ctx *part_inplace_ctx;
  int error = 0;

  DBUG_ENTER("Partition_base::commit_inplace_alter_table");

  /*
    Changing to similar partitioning, only update metadata.
    Non allowed changes would be catched in prep_alter_part_table().
  */
  if (ha_alter_info->alter_info->flags == Alter_info::ALTER_PARTITION)
    DBUG_RETURN(false);

  part_inplace_ctx = static_cast<class Partition_base_inplace_ctx *>(
      ha_alter_info->handler_ctx);

  if (commit) {
    DBUG_ASSERT(ha_alter_info->group_commit_ctx ==
                part_inplace_ctx->handler_ctx_array);

    // Call SE to drop partition/sub partitions
    if (ha_alter_info->alter_info->flags & Alter_info::ALTER_DROP_PARTITION) {
      handler **file = m_file;
      partition_info *part_info = ha_alter_info->modified_part_info;
      List_iterator_fast<partition_element> part_it(part_info->partitions);
      partition_element *part_elem;
      int num_subparts =
          part_info->is_sub_partitioned() ? part_info->num_subparts : 1;
      uint count = 0;
      while ((part_elem = part_it++) != nullptr) {
        partition_state state = part_elem->part_state;
        switch (state) {
          case PART_TO_BE_DROPPED:
            if (part_info->is_sub_partitioned()) {
              List_iterator<partition_element> sub_part_it(
                  part_elem->subpartitions);

              partition_element *sub_part_elem;
              while ((sub_part_elem = sub_part_it++) != nullptr) {
                char name[FN_REFLEN];
                create_subpartition_name(name, table->s->normalized_path.str,
                                         part_elem->partition_name,
                                         sub_part_elem->partition_name);
                error = (*file)->ha_external_lock(get_thd(), F_UNLCK);
                if (error) goto end;
                error = (*file)->ha_delete_table(name, old_table_def);
                if (error) goto end;
                (*file)->ha_close();
                destroy(*file);
                *file = nullptr;
                file++;
              }
            } else {
              char name[FN_REFLEN];
              create_partition_name(name, table->s->normalized_path.str,
                                    part_elem->partition_name, false);
              error = (*file)->ha_external_lock(get_thd(), F_UNLCK);
              if (error) goto end;
              error = (*file)->ha_delete_table(name, old_table_def);
              if (error) goto end;
              (*file)->ha_close();
              destroy(*file);
              *file = nullptr;
              file += num_subparts;
            }
            break;
          default:
            memmove(m_file + count, file, num_subparts * sizeof(handler *));
            file += num_subparts;
            count += num_subparts;
            break;
        }
      }
      m_file[count] = nullptr;
      m_tot_parts = count;
    } else if (ha_alter_info->alter_info->flags &
               Alter_info::ALTER_ADD_PARTITION) {
      partition_info *part_info = ha_alter_info->modified_part_info;
      List_iterator_fast<partition_element> part_it(part_info->partitions);
      partition_element *part_elem;
      int part_index = 0;
      // Allocate new partition handlers
      int num_subparts =
          part_info->is_sub_partitioned() ? part_info->num_subparts : 1;
      prepare_for_new_partitions(&altered_table->s->mem_root,
                                 part_info->get_tot_partitions());
      // Before creating new partitions check whether indexes are disabled
      // in the partitions.
      uint disable_non_uniq_indexes = indexes_are_disabled();
      while ((part_elem = part_it++) != nullptr) {
        partition_state state = part_elem->part_state;
        switch (state) {
          case PART_TO_BE_ADDED:
            if (part_info->is_sub_partitioned()) {
              List_iterator<partition_element> sub_part_it(
                  part_elem->subpartitions);

              partition_element *sub_part_elem;
              int sub_part_index = 0;
              while ((sub_part_elem = sub_part_it++) != nullptr) {
                // init sub parition handler
                m_new_file[part_index + sub_part_index] = get_file_handler(
                    altered_table->s, &(altered_table->s->mem_root));
                DBUG_EXECUTE_IF("fail_add_partition_1", {
                  m_new_partitions_name[part_index + sub_part_index] = nullptr;
                });
                // check memory
                if (m_new_file[part_index + sub_part_index] == nullptr) {
                  mem_alloc_error(sizeof(handler));
                  close_new_partitions(/* delete new partition */ true);
                  goto end;
                }
                m_new_file[part_index + sub_part_index]->set_ha_share_ref(
                    &altered_table->s->ha_share);
                // construct sub parition name
                char name[FN_REFLEN];
                create_subpartition_name(name, table->s->normalized_path.str,
                                         part_elem->partition_name,
                                         sub_part_elem->partition_name);
                // save sub partition name for recovery
                m_new_partitions_name[part_index + sub_part_index] =
                    (char *)altered_table->s->mem_root.Alloc(FN_REFLEN *
                                                             sizeof(char));
                if (m_new_partitions_name[part_index + sub_part_index] ==
                    nullptr) {
                  mem_alloc_error(FN_REFLEN * sizeof(char));
                  close_new_partitions(/* delete new partition */ true);
                  goto end;
                }
                strcpy(m_new_partitions_name[part_index + sub_part_index],
                       name);

                // create sub partition
                error = m_new_file[part_index + sub_part_index]->ha_create(
                    name, altered_table, ha_alter_info->create_info,
                    new_table_def);
                DBUG_EXECUTE_IF("fail_add_partition_2", {
                  error = HA_ERR_INTERNAL_ERROR;
                  my_error(error, MYF(0));
                });
                if (error) {
                  close_new_partitions(/* delete new partition */ true);
                  goto end;
                }
                error = m_new_file[part_index + sub_part_index]->ha_open(
                    altered_table, name, m_mode, HA_OPEN_IGNORE_IF_LOCKED,
                    new_table_def);
                DBUG_EXECUTE_IF("fail_add_partition_3", {
                  error = HA_ERR_INTERNAL_ERROR;
                  my_error(error, MYF(0));
                });
                if (error) {
                  close_new_partitions(/* delete new partition */ true);
                  goto end;
                }
                if (disable_non_uniq_indexes)
                  m_new_file[part_index + sub_part_index]->ha_disable_indexes(
                      HA_KEY_SWITCH_NONUNIQ_SAVE);
                sub_part_index++;
              }
            } else {
              // Init normal partition handle
              m_new_file[part_index] = get_file_handler(
                  altered_table->s, &(altered_table->s->mem_root));
              DBUG_EXECUTE_IF("fail_add_partition_1",
                              { m_new_file[part_index] = nullptr; });
              // check memory
              if (m_new_file[part_index] == nullptr) {
                mem_alloc_error(sizeof(handler));
                close_new_partitions(/* delete new partition */ true);
                goto end;
              }
              m_new_file[part_index]->set_ha_share_ref(
                  &altered_table->s->ha_share);
              // Create and init partition
              char name[FN_REFLEN];
              create_partition_name(name, table->s->normalized_path.str,
                                    part_elem->partition_name, false);
              // save parition name for error case
              m_new_partitions_name[part_index] =
                  (char *)altered_table->s->mem_root.Alloc(FN_REFLEN *
                                                           sizeof(char));
              if (m_new_partitions_name[part_index] == nullptr) {
                mem_alloc_error(FN_REFLEN * sizeof(char));
                close_new_partitions(/* delete new partition */ true);
                goto end;
              }
              strcpy(m_new_partitions_name[part_index], name);
              // create table
              error = m_new_file[part_index]->ha_create(
                  name, altered_table, ha_alter_info->create_info,
                  new_table_def);
              DBUG_EXECUTE_IF("fail_add_partition_2", {
                error = HA_ERR_INTERNAL_ERROR;
                my_error(error, MYF(0));
              });
              if (error) {
                close_new_partitions(/* delete new partition */ true);
                goto end;
              }
              error = m_new_file[part_index]->ha_open(
                  altered_table, name, m_mode, HA_OPEN_IGNORE_IF_LOCKED,
                  new_table_def);
              DBUG_EXECUTE_IF("fail_add_partition_3", {
                error = HA_ERR_INTERNAL_ERROR;
                my_error(error, MYF(0));
              });
              if (error) {
                close_new_partitions(/*delete new partition*/ true);
                goto end;
              }
              if (disable_non_uniq_indexes)
                m_new_file[part_index]->ha_disable_indexes(
                    HA_KEY_SWITCH_NONUNIQ_SAVE);
            }
            part_index += num_subparts;
            break;
          default:
            part_index += num_subparts;
            break;
        }
      }
    }

    ha_alter_info->handler_ctx = part_inplace_ctx->handler_ctx_array[0];
    error = m_file[0]->ha_commit_inplace_alter_table(
        altered_table, ha_alter_info, commit, old_table_def, new_table_def);
    if (error) goto end;
    if (ha_alter_info->group_commit_ctx) {
      /*
        If ha_alter_info->group_commit_ctx is not set to nullptr,
        then the engine did only commit the first partition!
        The engine is probably new, since both innodb and the default
        implementation of handler::commit_inplace_alter_table sets it to nullptr
        and simply return false, since it allows metadata changes only.
        Loop over all other partitions as to follow the protocol!
      */
      uint i;
      DBUG_ASSERT(0);
      for (i = 1; i < m_tot_parts; i++) {
        ha_alter_info->handler_ctx = part_inplace_ctx->handler_ctx_array[i];
        error |= m_file[i]->ha_commit_inplace_alter_table(
            altered_table, ha_alter_info, true, old_table_def, new_table_def);
      }
    }
  } else {
    uint i;
    for (i = 0; i < m_tot_parts; i++) {
      /* Rollback, commit == false,  is done for each partition! */
      ha_alter_info->handler_ctx = part_inplace_ctx->handler_ctx_array[i];
      if (m_file[i]->ha_commit_inplace_alter_table(altered_table, ha_alter_info,
                                                   false, old_table_def,
                                                   new_table_def))
        error = true;
    }
  }
end:
  ha_alter_info->handler_ctx = part_inplace_ctx;

  DBUG_RETURN(error);
}

void Partition_base::notify_table_changed(Alter_inplace_info *aii) {
  handler **file;

  DBUG_ENTER("Partition_base::notify_table_changed");

  for (file = m_file; *file; file++) (*file)->ha_notify_table_changed(aii);

  DBUG_VOID_RETURN;
}

uint Partition_base::max_supported_key_parts() const {
  if (m_file) return m_file[0]->max_supported_key_parts();
  return std::unique_ptr<handler, Destroy_only<handler>>(
             get_file_handler(nullptr, ha_thd()->mem_root))
      ->max_supported_key_parts();
}

uint Partition_base::max_supported_key_length() const {
  if (m_file) return m_file[0]->max_supported_key_length();
  return std::unique_ptr<handler, Destroy_only<handler>>(
             get_file_handler(nullptr, ha_thd()->mem_root))
      ->max_supported_key_length();
}

uint Partition_base::max_supported_key_part_length(
    HA_CREATE_INFO *create_info) const {
  if (m_file) return m_file[0]->max_supported_key_part_length(create_info);
  return std::unique_ptr<handler, Destroy_only<handler>>(
             get_file_handler(nullptr, ha_thd()->mem_root))
      ->max_supported_key_part_length(create_info);
}

uint Partition_base::max_supported_record_length() const {
  if (m_file) return m_file[0]->max_supported_record_length();
  return std::unique_ptr<handler, Destroy_only<handler>>(
             get_file_handler(nullptr, ha_thd()->mem_root))
      ->max_supported_record_length();
}

uint Partition_base::max_supported_keys() const {
  if (m_file) return m_file[0]->max_supported_keys();
  return std::unique_ptr<handler, Destroy_only<handler>>(
             get_file_handler(nullptr, ha_thd()->mem_root))
      ->max_supported_keys();
}

uint Partition_base::extra_rec_buf_length() const {
  if (!m_file)
    return std::unique_ptr<handler, Destroy_only<handler>>(
               get_file_handler(nullptr, ha_thd()->mem_root))
        ->extra_rec_buf_length();

  uint max = (*m_file)->extra_rec_buf_length();

  handler **file;
  for (file = m_file, file++; *file; file++)
    if (max < (*file)->extra_rec_buf_length())
      max = (*file)->extra_rec_buf_length();
  return max;
}

uint Partition_base::min_record_length(uint options) const {
  if (!m_file)
    return std::unique_ptr<handler, Destroy_only<handler>>(
               get_file_handler(nullptr, ha_thd()->mem_root))
        ->min_record_length(options);

  uint max = (*m_file)->min_record_length(options);

  handler **file;
  for (file = m_file, file++; *file; file++)
    if (max < (*file)->min_record_length(options))
      max = (*file)->min_record_length(options);
  return max;
}

/****************************************************************************
                MODULE compare records
****************************************************************************/
/*
  Compare two positions

  SYNOPSIS
    cmp_ref()
    ref1                   First position
    ref2                   Second position

  RETURN VALUE
    <0                     ref1 < ref2
    0                      Equal
    >0                     ref1 > ref2

  DESCRIPTION
    We get two references and need to check if those records are the same.
    If they belong to different partitions we decide that they are not
    the same record. Otherwise we use the particular handler to decide if
    they are the same. Sort in partition id order if not equal.
*/

int Partition_base::cmp_ref(const uchar *ref1, const uchar *ref2) const {
  int cmp;
  ptrdiff_t diff1, diff2;
  DBUG_ENTER("Partition_base::cmp_ref");

  cmp = m_file[0]->cmp_ref((ref1 + PARTITION_BYTES_IN_POS),
                           (ref2 + PARTITION_BYTES_IN_POS));
  if (cmp) DBUG_RETURN(cmp);

  if ((ref1[0] == ref2[0]) && (ref1[1] == ref2[1])) {
    /* This means that the references are same and are in same partition.*/
    DBUG_RETURN(0);
  }

  diff1 = ref2[1] - ref1[1];
  diff2 = ref2[0] - ref1[0];
  if (diff1 > 0) {
    DBUG_RETURN(-1);
  }
  if (diff1 < 0) {
    DBUG_RETURN(+1);
  }
  if (diff2 > 0) {
    DBUG_RETURN(-1);
  }
  DBUG_RETURN(+1);
}

/****************************************************************************
                MODULE condition pushdown
****************************************************************************/

/**
  Index condition pushdown registation
  @param keyno     Key number for the condition
  @param idx_cond  Item tree of the condition to test

  @return Remainder of non handled condition

  @note Only handles full condition or nothing at all. MyISAM and InnoDB
  both only supports full or nothing.
*/
Item *Partition_base::idx_cond_push(uint keyno, Item *idx_cond) {
  uint i;
  Item *res;
  DBUG_ENTER("Partition_base::idx_cond_push");
  DBUG_EXECUTE("where", print_where(get_thd(), idx_cond, "cond", QT_ORDINARY););
  DBUG_PRINT("info", ("keyno: %u, active_index: %u", keyno, active_index));
  DBUG_ASSERT(pushed_idx_cond == nullptr);

  for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
       i = m_part_info->get_next_used_partition(i)) {
    res = m_file[i]->idx_cond_push(keyno, idx_cond);
    if (res) {
      uint j;
      /*
        All partitions has the same structure, so if the first partition
        succeeds, then the rest will also succeed.
      */
      DBUG_ASSERT(i == m_part_info->get_first_used_partition());
      /* Only supports entire index conditions or no conditions! */
      DBUG_ASSERT(res == idx_cond);
      if (res != idx_cond) m_file[i]->cancel_pushed_idx_cond();
      /* cancel previous calls. */
      for (j = m_part_info->get_first_used_partition();
           j < i;  // No need for cancel i, since no support
           j = m_part_info->get_next_used_partition(j)) {
        m_file[j]->cancel_pushed_idx_cond();
      }
      DBUG_RETURN(idx_cond);
    }
  }
  DBUG_ASSERT(pushed_idx_cond_keyno == MAX_KEY);
  pushed_idx_cond = idx_cond;
  pushed_idx_cond_keyno = keyno;
  DBUG_PRINT("info", ("Index condition pushdown used for keyno: %u", keyno));
  DBUG_RETURN(nullptr);
}

/** Reset information about pushed index conditions */
void Partition_base::cancel_pushed_idx_cond() {
  uint i;
  DBUG_ENTER("Partition_base::cancel_pushed_idx_cond");
  if (pushed_idx_cond) {
    for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
         i = m_part_info->get_next_used_partition(i)) {
      m_file[i]->cancel_pushed_idx_cond();
    }
    pushed_idx_cond = nullptr;
    pushed_idx_cond_keyno = MAX_KEY;
  }

  DBUG_VOID_RETURN;
}

/****************************************************************************
                MODULE auto increment
****************************************************************************/

/**
  Initialize the shared auto increment value.

  @param no_lock  If HA_STATUS_NO_LOCK should be used in info(HA_STATUS_AUTO).

  Also sets stats.auto_increment_value.
*/

inline int Partition_base::initialize_auto_increment(bool no_lock) {
  DBUG_ENTER("Partition_base::initialize_auto_increment");
#ifndef DBUG_OFF
  if (table_share->tmp_table == NO_TMP_TABLE) {
    mysql_mutex_assert_owner(part_share->auto_inc_mutex);
  }
#endif
  DBUG_ASSERT(!part_share->auto_inc_initialized);

  /*
    The auto-inc mutex in the table_share is locked, so we do not need
    to have the handlers locked.
    HA_STATUS_NO_LOCK is not checked, since we cannot skip locking
    the mutex, because it is initialized.
  */
  handler *file, **file_array;
  ulonglong auto_increment_value = 0;
  uint no_lock_flag = no_lock ? HA_STATUS_NO_LOCK : 0;
  int ret_error, error = 0;
  file_array = m_file;
  DBUG_PRINT("info", ("checking all partitions for auto_increment_value"));
  do {
    file = *file_array;
    ret_error = file->info(HA_STATUS_AUTO | no_lock_flag);
    auto_increment_value =
        std::max(auto_increment_value, file->stats.auto_increment_value);
    if (ret_error && !error) {
      error = ret_error;
    }
  } while (*(++file_array));

  DBUG_ASSERT(auto_increment_value);
  stats.auto_increment_value = auto_increment_value;
  /*
    We only use the cached auto inc value if it is
    the first part of the key.
  */
  if (table_share->next_number_keypart == 0) {
    DBUG_ASSERT(part_share->next_auto_inc_val <= auto_increment_value);
    part_share->next_auto_inc_val = auto_increment_value;
    part_share->auto_inc_initialized = true;
    DBUG_PRINT("info", ("initializing next_auto_inc_val to %lu",
                        (ulong)part_share->next_auto_inc_val));
  }
  DBUG_RETURN(error);
}

/**
  This method is called by update_auto_increment which in turn is called
  by the individual handlers as part of write_row. We use the
  part_share->next_auto_inc_val, or search all
  partitions for the highest auto_increment_value if not initialized or
  if auto_increment field is a secondary part of a key, we must search
  every partition when holding a mutex to be sure of correctness.
*/

void Partition_base::get_auto_increment(ulonglong offset, ulonglong increment,
                                        ulonglong nb_desired_values,
                                        ulonglong *first_value,
                                        ulonglong *nb_reserved_values) {
  DBUG_ENTER("Partition_base::get_auto_increment");
  DBUG_PRINT("info", ("offset: %lu inc: %lu desired_values: %lu "
                      "first_value: %lu",
                      (ulong)offset, (ulong)increment, (ulong)nb_desired_values,
                      (ulong)*first_value));
  DBUG_ASSERT(increment && nb_desired_values);
  *first_value = 0;
  if (table->s->next_number_keypart) {
    /*
      next_number_keypart is != 0 if the auto_increment column is a secondary
      column in the index (it is allowed in MyISAM)
    */
    DBUG_PRINT("info", ("next_number_keypart != 0"));
    ulonglong nb_reserved_values_part;
    ulonglong first_value_part, max_first_value;
    handler **file = m_file;
    first_value_part = max_first_value = *first_value;
    /* Must lock and find highest value among all partitions. */
    lock_auto_increment();
    do {
      /* Only nb_desired_values = 1 makes sense */
      (*file)->get_auto_increment(offset, increment, 1, &first_value_part,
                                  &nb_reserved_values_part);
      if (first_value_part == ULLONG_MAX)  // error in one partition
      {
        *first_value = first_value_part;
        /* log that the error was between table/partition handler */
        sql_print_error("Partition failed to reserve auto_increment value");
        unlock_auto_increment();
        DBUG_VOID_RETURN;
      }
      DBUG_PRINT("info", ("first_value_part: %lu", (ulong)first_value_part));
      max_first_value = std::max(max_first_value, first_value_part);
    } while (*(++file));
    *first_value = max_first_value;
    *nb_reserved_values = 1;
    unlock_auto_increment();
  } else {
    Partition_helper::get_auto_increment_first_field(
        increment, nb_desired_values, first_value, nb_reserved_values);
  }
  DBUG_VOID_RETURN;
}

void Partition_base::release_auto_increment_all_parts() {
  uint i;
  DBUG_ENTER("Partition_base::release_auto_increment_all_parts");

  DBUG_ASSERT(table->s->next_number_keypart);
  for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
       i = bitmap_get_next_set(&m_part_info->lock_partitions, i)) {
    m_file[i]->ha_release_auto_increment();
  }
  DBUG_VOID_RETURN;
}

/****************************************************************************
                MODULE initialize handler for HANDLER call
****************************************************************************/

void Partition_base::init_table_handle_for_HANDLER() {
  uint i;
  for (i = m_part_info->get_first_used_partition(); i < m_tot_parts;
       i = m_part_info->get_next_used_partition(i))
    m_file[i]->init_table_handle_for_HANDLER();
  return;
}

/**
  Return the checksum of the partition.

  @param part_id Partition to checksum.

  @return Checksum or 0 if not supported.
*/

ha_checksum Partition_base::checksum_in_part(uint part_id) const {
  if ((table_flags() & HA_HAS_CHECKSUM)) {
    return m_file[part_id]->checksum();
  }
  return 0;
}

/****************************************************************************
                MODULE enable/disable indexes
****************************************************************************/

/*
  Disable indexes for a while
  SYNOPSIS
    disable_indexes()
    mode                      Mode
  RETURN VALUES
    0                         Success
    != 0                      Error
*/

int Partition_base::disable_indexes(uint mode) {
  handler **file;
  int error = 0;

  DBUG_ASSERT(bitmap_is_set_all(&(m_part_info->lock_partitions)));
  for (file = m_file; *file; file++) {
    if ((error = (*file)->ha_disable_indexes(mode))) break;
  }
  return error;
}

/*
  Enable indexes again
  SYNOPSIS
    enable_indexes()
    mode                      Mode
  RETURN VALUES
    0                         Success
    != 0                      Error
*/

int Partition_base::enable_indexes(uint mode) {
  handler **file;
  int error = 0;

  DBUG_ASSERT(bitmap_is_set_all(&(m_part_info->lock_partitions)));
  for (file = m_file; *file; file++) {
    if ((error = (*file)->ha_enable_indexes(mode))) break;
  }
  return error;
}

/*
  Check if indexes are disabled
  SYNOPSIS
    indexes_are_disabled()

  RETURN VALUES
    0                      Indexes are enabled
    != 0                   Indexes are disabled
*/

int Partition_base::indexes_are_disabled(void) {
  handler **file;
  int error = 0;

  DBUG_ASSERT(bitmap_is_set_all(&(m_part_info->lock_partitions)));
  for (file = m_file; *file; file++) {
    if ((error = (*file)->indexes_are_disabled())) break;
  }
  return error;
}

int Partition_base::check_for_upgrade(HA_CHECK_OPT *check_opt) {
  int error = HA_ADMIN_NEEDS_CHECK;
  DBUG_ENTER("Partition_base::check_for_upgrade");

  /*
    This is called even without FOR UPGRADE,
    if the .frm version is lower than the current version.
    In that case return that it needs checking!
  */
  if (!(check_opt->sql_flags & TT_FOR_UPGRADE)) {
    DBUG_RETURN(error);
  }

  DBUG_RETURN(error);
}

/*
  -------------------------------------------------------------------------
  MODULE MRR support
  -------------------------------------------------------------------------
*/
ha_rows Partition_base::multi_range_read_info_const(
    uint keyno, RANGE_SEQ_IF *seq, void *seq_init_param, uint n_ranges,
    uint *bufsz, uint *flags, Cost_estimate *cost) {
  ha_rows rows = 0, estimated_rows = 0;
  uint partition_index = 0, part_id;
  uint default_bufsz = *bufsz;
  DBUG_ENTER("Partition_base::multi_range_read_info_const");

  while ((part_id = get_biggest_used_partition(&partition_index)) !=
         NO_CURRENT_PART_ID) {
    uint part_bufsz = default_bufsz;
    Cost_estimate part_cost;
    rows = m_file[part_id]->multi_range_read_info_const(
        keyno, seq, seq_init_param, n_ranges, &part_bufsz, flags, &part_cost);
    if (rows == HA_POS_ERROR) DBUG_RETURN(HA_POS_ERROR);

    DBUG_PRINT("info", ("part %u match %lu rows of %lu", part_id, (ulong)rows,
                        (ulong)m_file[part_id]->stats.records));
    *cost += part_cost;
    estimated_rows += rows;
    *bufsz = max(*bufsz, part_bufsz);
  }

  *flags &= ~HA_MRR_SUPPORT_SORTED;  //  Non-sorted mode
  DBUG_RETURN(estimated_rows);
}

ha_rows Partition_base::multi_range_read_info(uint keyno, uint n_ranges,
                                              uint keys, uint *bufsz,
                                              uint *flags,
                                              Cost_estimate *cost) {
  ha_rows rows = 0, estimated_rows = 0;
  uint partition_index = 0, part_id;
  Cost_estimate part_cost;
  DBUG_ENTER("Partition_base::multi_range_read_info");

  if ((part_id = get_biggest_used_partition(&partition_index)) !=
      NO_CURRENT_PART_ID) {
    rows = m_file[part_id]->multi_range_read_info(keyno, n_ranges, keys, bufsz,
                                                  flags, &part_cost);

    if (rows == HA_POS_ERROR) DBUG_RETURN(HA_POS_ERROR);
    DBUG_PRINT("info", ("part %u match %lu rows of %lu", part_id, (ulong)rows,
                        (ulong)m_file[part_id]->stats.records));
  }
  uint tot_used_partitions = m_part_info->num_partitions_used();
  part_cost.multiply(tot_used_partitions);
  *cost += part_cost;
  estimated_rows += rows * tot_used_partitions;
  DBUG_RETURN(estimated_rows);
}

int Partition_base::multi_range_read_init(RANGE_SEQ_IF *seq,
                                          void *seq_init_param, uint n_ranges,
                                          uint mode, HANDLER_BUFFER *buf) {
  DBUG_ENTER("Partition_base::multi_range_read_init");
  mrr_have_range = false;

  if (m_ordered) {
    mrr_uses_default_impl = true;
    DBUG_RETURN(handler::multi_range_read_init(seq, seq_init_param, n_ranges,
                                               mode, buf));
  }
  // Only initialze the first used partition
  uint part_id = m_part_info->get_first_used_partition();
  if (part_id == MY_BIT_NONE) {
    /* No partition to scan. */
    return HA_ERR_END_OF_FILE;
  }
  int res = m_file[part_id]->multi_range_read_init(seq, seq_init_param,
                                                   n_ranges, mode, buf);

  if (res != 0) DBUG_RETURN(res);

  if (m_ordered || m_file[part_id]->mrr_uses_default_impl) {
    mrr_uses_default_impl = true;
    res = handler::multi_range_read_init(seq, seq_init_param, n_ranges, mode,
                                         buf);
    DBUG_RETURN(res);
  }
  mrr_uses_default_impl = false;
  // Save multi_range_read_init parameters to initialize following partitions
  // During m_file[part_id]->multi_range_read_init(),
  // it will call quick_range_seq_init() to init seq_init_param fields.---
  // seq_init_param->qr_traversal_ctx.cur ==
  // seq_init_param->qr_traversal_ctx.begin
  //
  // During m_file[part_id]->multi_range_read_next(), it will finish scan all
  // ranges in that partition --- seq_init_param->qr_traversal_ctx.cur ==
  // seq_init_param->qr_traversal_ctx.end.
  // also all partitions in a table share same seq_init_param which is a
  // QUICK_RANGE_SELECT instance.
  // The following m_file[next part_id]->multi_range_read_next() will do
  // nothing without re(initialze) seq_init_param->qr_traversal_ctx.cur
  m_mrr_seq = *seq;
  m_mrr_seq_init_param = seq_init_param;
  m_mrr_n_ranges = n_ranges;
  m_mrr_mode = mode;
  m_mrr_buf = buf;

  m_part_spec.start_part = part_id;
  m_part_spec.end_part = m_tot_parts - 1;
  DBUG_RETURN(res);
}

int Partition_base::multi_range_read_next(char **range_info) {
  int res = 0;
  DBUG_ENTER("Partition_base::multi_range_read_next");

  if (mrr_uses_default_impl) {
    DBUG_RETURN(handler::multi_range_read_next(range_info));
  }

  // SE multi_range_read_next support kill switch to avoid infinite loop
  while (true) {
    res = m_file[m_part_spec.start_part]->ha_multi_range_read_next(range_info);
    if (res == HA_ERR_END_OF_FILE) {
      // find next partition
      m_part_spec.start_part =
          m_part_info->get_next_used_partition(m_part_spec.start_part);
      // no more part?
      if (m_part_spec.start_part >= m_tot_parts) {
        DBUG_RETURN(HA_ERR_END_OF_FILE);
      }

      // init next partition
      m_file[m_part_spec.start_part]->multi_range_read_init(
          &m_mrr_seq, m_mrr_seq_init_param, m_mrr_n_ranges, m_mrr_mode,
          m_mrr_buf);
    } else {
      break;
    }
  }
  DBUG_RETURN(res);
}

}  // namespace native_part
