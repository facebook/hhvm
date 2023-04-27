/* Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_info_factory.h"
#include "dependency_slave_worker.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_base.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/mysqld.h"  // key_master_info_run_lock
#include "sql/rpl_filter.h"
#include "sql/rpl_info.h"
#include "sql/rpl_info_dummy.h"         // Rpl_info_dummy
#include "sql/rpl_info_file.h"          // Rpl_info_file
#include "sql/rpl_info_table.h"         // Rpl_info_table
#include "sql/rpl_info_table_access.h"  // Rpl_info_table_access
#include "sql/rpl_mi.h"                 // Master_info
#include "sql/rpl_msr.h"                // channel_map
#include "sql/rpl_rli.h"                // Relay_log_info
#include "sql/rpl_rli_pdb.h"            // Slave_worker
#include "sql/rpl_slave.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql_string.h"
#include "thr_lock.h"

/*
  Defines meta information on diferent repositories.
*/
Rpl_info_factory::struct_table_data Rpl_info_factory::rli_table_data;
Rpl_info_factory::struct_file_data Rpl_info_factory::rli_file_data;
Rpl_info_factory::struct_table_data Rpl_info_factory::mi_table_data;
Rpl_info_factory::struct_file_data Rpl_info_factory::mi_file_data;
Rpl_info_factory::struct_file_data Rpl_info_factory::worker_file_data;
Rpl_info_factory::struct_table_data Rpl_info_factory::worker_table_data;

/**
  Creates a Master info repository whose type is defined as a parameter.

  @param[in]  mi_option Type of the repository, e.g. FILE TABLE.
  @param[in]  channel   the channel for which mi is to be created
  @param[in]  to_decide_repo     the flag is set to true if mi repositories
                                 are allowed to convert. For details,
                                 see init_slave()

  The execution fails if a user requests a type but a different type
  already exists in the system. This is done to avoid that a user
  accidentally accesses the wrong repository and makes the slave go out
  of sync.

  @retval Pointer to Master_info Success
  @retval NULL  Failure
*/
Master_info *Rpl_info_factory::create_mi(uint mi_option, const char *channel,
                                         bool to_decide_repo) {
  Master_info *mi = nullptr;
  Rpl_info_handler *handler_src = nullptr;
  Rpl_info_handler *handler_dest = nullptr;
  uint instances = 1;
  const char *msg =
      "Failed to allocate memory for the master info "
      "structure";

  DBUG_TRACE;

  if (!(mi = new Master_info(
#ifdef HAVE_PSI_INTERFACE
            &key_master_info_run_lock, &key_master_info_data_lock,
            &key_master_info_sleep_lock, &key_master_info_thd_lock,
            &key_master_info_rotate_lock, &key_master_info_data_cond,
            &key_master_info_start_cond, &key_master_info_stop_cond,
            &key_master_info_sleep_cond, &key_master_info_rotate_cond,
#endif
            instances, channel)))
    goto err;

  if (init_repositories(mi_table_data, mi_file_data, mi_option, &handler_src,
                        &handler_dest, &msg))
    goto err;

  if (to_decide_repo) {
    if (decide_repository(mi, mi_option, &handler_src, &handler_dest, &msg))
      goto err;
  } else {
    if (handler_dest->get_rpl_info_type() != INFO_REPOSITORY_TABLE) {
      LogErr(ERROR_LEVEL, ER_RPL_REPO_SHOULD_BE_TABLE);
      goto err;
    }
    mi->set_rpl_info_handler(handler_dest);

    if (mi->set_info_search_keys(handler_dest)) goto err;

    delete handler_src;
  }

  return mi;

err:
  delete handler_src;
  delete handler_dest;
  if (mi) {
    /*
      The handler was previously deleted so we need to remove
      any reference to it.
    */
    mi->set_rpl_info_handler(nullptr);
    mi->channel_wrlock();
    delete mi;
  }
  LogErr(ERROR_LEVEL, ER_RPL_ERROR_CREATING_MASTER_INFO, msg);
  return nullptr;
}

/**
  Allows to change the master info repository after startup.

  @param[in]  mi        Reference to Master_info.
  @param[in]  mi_option Type of the repository, e.g. FILE TABLE.
  @param[out] msg       Error message if something goes wrong.

  @retval false No error
  @retval true  Failure
*/
bool Rpl_info_factory::change_mi_repository(Master_info *mi, uint mi_option,
                                            const char **msg) {
  Rpl_info_handler *handler_src = mi->get_rpl_info_handler();
  Rpl_info_handler *handler_dest = nullptr;
  DBUG_TRACE;

  DBUG_ASSERT(handler_src);

  if (init_repositories(mi_table_data, mi_file_data, mi_option, nullptr,
                        &handler_dest, msg))
    goto err;

  if (decide_repository(mi, mi_option, &handler_src, &handler_dest, msg))
    goto err;

  return false;

err:
  delete handler_dest;
  handler_dest = nullptr;

  LogErr(ERROR_LEVEL, ER_RPL_ERROR_CHANGING_MASTER_INFO_REPO_TYPE, *msg);
  return true;
}

/**
  Creates a Relay log info repository whose type is defined as a parameter.

  @param[in]  rli_option        Type of the Relay log info repository
  @param[in]  is_slave_recovery If the slave should try to start a recovery
                                process to get consistent relay log files
  @param[in]  channel   the channel for which mi is to be created
  @param[in]  to_decide_repo    If true, rli repositories are allowed
                               to convert from one repo to other


  The execution fails if a user requests a type but a different type
  already exists in the system. This is done to avoid that a user
  accidentally accesses the wrong repository and make the slave go out
  of sync.

  @retval Pointer to Relay_log_info Success
  @retval NULL  Failure
*/
Relay_log_info *Rpl_info_factory::create_rli(uint rli_option,
                                             bool is_slave_recovery,
                                             const char *channel,
                                             bool to_decide_repo) {
  Relay_log_info *rli = nullptr;
  Rpl_info_handler *handler_src = nullptr;
  Rpl_info_handler *handler_dest = nullptr;
  uint instances = 1;
  uint worker_repository = INVALID_INFO_REPOSITORY;
  uint worker_instances = 1;
  const char *msg = nullptr;
  const char *msg_alloc =
      "Failed to allocate memory for the relay log info "
      "structure";
  Rpl_filter *rpl_filter = nullptr;

  DBUG_TRACE;

  /*
    Returns how many occurrences of worker's repositories exist. For example,
    if the repository is a table, this retrieves the number of rows in it.
    Besides, it also returns the type of the repository where entries were
    found.
  */
  if (rli_option != INFO_REPOSITORY_DUMMY &&
      scan_repositories(&worker_instances, &worker_repository,
                        worker_table_data, worker_file_data, &msg))
    goto err;

  if (!(rli = new Relay_log_info(
            is_slave_recovery,
#ifdef HAVE_PSI_INTERFACE
            &key_relay_log_info_run_lock, &key_relay_log_info_data_lock,
            &key_relay_log_info_sleep_lock, &key_relay_log_info_thd_lock,
            &key_relay_log_info_data_cond, &key_relay_log_info_start_cond,
            &key_relay_log_info_stop_cond, &key_relay_log_info_sleep_cond,
#endif
            instances, channel,
            (rli_option != INFO_REPOSITORY_TABLE &&
             rli_option != INFO_REPOSITORY_FILE)))) {
    msg = msg_alloc;
    goto err;
  }

  rli->populate_recovery_binlog_max_gtid();

  if (init_repositories(rli_table_data, rli_file_data, rli_option, &handler_src,
                        &handler_dest, &msg))
    goto err;

  if (rli_option != INFO_REPOSITORY_DUMMY &&
      worker_repository != INVALID_INFO_REPOSITORY &&
      worker_repository != rli_option) {
    opt_rli_repository_id = rli_option = worker_repository;
    LogErr(WARNING_LEVEL,
           ER_RPL_CHANGING_RELAY_LOG_INFO_REPO_TYPE_FAILED_DUE_TO_GAPS);
    std::swap(handler_src, handler_dest);
  }

  if (to_decide_repo) {
    if (decide_repository(rli, rli_option, &handler_src, &handler_dest, &msg))
      goto err;
  } else {
    if (channel != nullptr) {
      /* Here dest code should be TABLE type repo. See, init_slave() */
      if (handler_dest->get_rpl_info_type() != INFO_REPOSITORY_TABLE) {
        LogErr(ERROR_LEVEL, ER_RPL_REPO_SHOULD_BE_TABLE);
        goto err;
      }

      if (rli->set_info_search_keys(handler_dest)) goto err;
    }

    rli->set_rpl_info_handler(handler_dest);

    /*
      By this time, rli must be loaded with it's primary key,
      which is channel_name
    */
    delete handler_src;
    handler_src = nullptr;
  }

  /* Set filters here to guarantee that any rli object has a valid filter */
  rpl_filter = rpl_channel_filters.get_channel_filter(channel);
  if (rpl_filter == nullptr) {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_FILTER_CREATE_FAILED, channel);
    msg = msg_alloc;
    goto err;
  }
  rli->set_filter(rpl_filter);
  rpl_filter->set_attached();

  return rli;

err:
  delete handler_src;
  delete handler_dest;
  if (rli) {
    /*
      The handler was previously deleted so we need to remove
      any reference to it.
    */
    rli->set_rpl_info_handler(nullptr);
    delete rli;
  }
  LogErr(ERROR_LEVEL, ER_RPL_ERROR_CREATING_RELAY_LOG_INFO, msg);
  return nullptr;
}

/**
  Allows to change the relay log info repository after startup.

  @param[in]  rli       Pointer to Relay_log_info.
  @param[in]  rli_option Type of the repository, e.g. FILE TABLE.
  @param[out] msg       Error message if something goes wrong.

  @retval false No error
  @retval true  Failure
*/
bool Rpl_info_factory::change_rli_repository(Relay_log_info *rli,
                                             uint rli_option,
                                             const char **msg) {
  Rpl_info_handler *handler_src = rli->get_rpl_info_handler();
  Rpl_info_handler *handler_dest = nullptr;
  DBUG_TRACE;

  DBUG_ASSERT(handler_src != nullptr);

  if (init_repositories(rli_table_data, rli_file_data, rli_option, nullptr,
                        &handler_dest, msg))
    goto err;

  if (decide_repository(rli, rli_option, &handler_src, &handler_dest, msg))
    goto err;

  return false;

err:
  delete handler_dest;
  handler_dest = nullptr;

  LogErr(ERROR_LEVEL, ER_RPL_ERROR_CHANGING_RELAY_LOG_INFO_REPO_TYPE, *msg);
  return true;
}

/**
   Delete all info from Worker info tables to render them useless in
   future MTS recovery, and indicate that in Coordinator info table.

   @retval false on success
   @retval true when a failure in deletion or writing to Coordinator table
   fails.
*/
bool Rpl_info_factory::reset_workers(Relay_log_info *rli) {
  bool error = true;

  DBUG_TRACE;

  if (rli->recovery_parallel_workers == 0) return false;

  if (Rpl_info_file::do_reset_info(
          Slave_worker::get_number_worker_fields(), worker_file_data.pattern,
          worker_file_data.name_indexed, &worker_file_data.nullable_fields))
    goto err;

  if (Rpl_info_table::do_reset_info(Slave_worker::get_number_worker_fields(),
                                    MYSQL_SCHEMA_NAME.str, WORKER_INFO_NAME.str,
                                    rli->channel,
                                    &worker_file_data.nullable_fields))
    goto err;

  error = false;

  DBUG_EXECUTE_IF("mts_debug_reset_workers_fails", error = true;);

err:
  if (error)
    LogErr(ERROR_LEVEL,
           ER_RPL_FAILED_TO_DELETE_FROM_SLAVE_WORKERS_INFO_REPOSITORY);
  rli->recovery_parallel_workers = 0;
  rli->clear_mts_recovery_groups();
  if (rli->flush_info(true)) {
    error = true;
    LogErr(ERROR_LEVEL, ER_RPL_FAILED_TO_RESET_STATE_IN_SLAVE_INFO_REPOSITORY);
  }
  return error;
}

/**
  Creates a Slave worker repository whose type is defined as a parameter.

  @param[in]  rli_option Type of the repository, e.g. FILE TABLE.
  @param[in]  worker_id  ID of the worker to be created.
  @param[in]  rli        Pointer to Relay_log_info.
  @param[in]  is_gaps_collecting_phase See Slave_worker::rli_init_info

  The execution fails if a user requests a type but a different type
  already exists in the system. This is done to avoid that a user
  accidentally accesses the wrong repository and make the slave go out
  of sync.

  @retval Pointer to Slave_worker Success
  @retval NULL  Failure
*/
Slave_worker *Rpl_info_factory::create_worker(uint rli_option, uint worker_id,
                                              Relay_log_info *rli,
                                              bool is_gaps_collecting_phase) {
  Rpl_info_handler *handler_src = nullptr;
  Rpl_info_handler *handler_dest = nullptr;
  Slave_worker *worker = nullptr;
  const char *msg =
      "Failed to allocate memory for the worker info "
      "structure";

  DBUG_TRACE;

  /*
    Define the name of the worker and its repository.
  */
  char *pos = my_stpcpy(worker_file_data.name, worker_file_data.pattern);
  sprintf(pos, "%u", worker_id + 1);

  if (get_mts_parallel_option() == MTS_PARALLEL_TYPE_DEPENDENCY)
    worker = new Dependency_slave_worker(
        rli,
#ifdef HAVE_PSI_INTERFACE
        &key_relay_log_info_run_lock, &key_relay_log_info_data_lock,
        &key_relay_log_info_sleep_lock, &key_relay_log_info_thd_lock,
        &key_relay_log_info_data_cond, &key_relay_log_info_start_cond,
        &key_relay_log_info_stop_cond, &key_relay_log_info_sleep_cond,
#endif
        worker_id, rli->get_channel());
  else
    worker = new Slave_worker(
        rli,
#ifdef HAVE_PSI_INTERFACE
        &key_relay_log_info_run_lock, &key_relay_log_info_data_lock,
        &key_relay_log_info_sleep_lock, &key_relay_log_info_thd_lock,
        &key_relay_log_info_data_cond, &key_relay_log_info_start_cond,
        &key_relay_log_info_stop_cond, &key_relay_log_info_sleep_cond,
#endif
        worker_id, rli->get_channel());

  if (!worker) goto err;

  if (init_repositories(worker_table_data, worker_file_data, rli_option,
                        &handler_src, &handler_dest, &msg))
    goto err;
  /*
    Preparing the being set up handler with search keys early.
    The file repo type handler can't be manupulated this way and it does
    not have to.
  */
  if (handler_dest->get_rpl_info_type() == INFO_REPOSITORY_TABLE)
    worker->set_info_search_keys(handler_dest);

  /* get_num_instances() requires channel_map lock */
  /*
  DBUG_ASSERT(channel_map.get_num_instances() <= 1 ||
              (rli_option == 1 && handler_dest->get_rpl_info_type() == 1));
  */
  if (decide_repository(worker, rli_option, &handler_src, &handler_dest, &msg))
    goto err;

  if (DBUG_EVALUATE_IF("mts_worker_thread_init_fails", 1, 0) ||
      worker->rli_init_info(is_gaps_collecting_phase)) {
    DBUG_EXECUTE_IF("enable_mts_worker_failure_init", {
      DBUG_SET("-d,mts_worker_thread_init_fails");
      DBUG_SET("-d,enable_mts_worker_failure_init");
    });
    DBUG_EXECUTE_IF("enable_mts_wokrer_failure_in_recovery_finalize", {
      DBUG_SET("-d,mts_worker_thread_init_fails");
      DBUG_SET("-d,enable_mts_wokrer_failure_in_recovery_finalize");
    });
    msg = "Failed to initialize the worker info structure";
    goto err;
  }

  if (rli->info_thd && rli->info_thd->is_error()) {
    msg = "Failed to initialize worker info table";
    goto err;
  }
  return worker;

err:
  delete handler_src;
  delete handler_dest;
  if (worker) {
    /*
      The handler was previously deleted so we need to remove
      any reference to it.
    */
    worker->set_rpl_info_handler(nullptr);
    delete worker;
  }
  LogErr(ERROR_LEVEL, ER_RPL_ERROR_CREATING_RELAY_LOG_INFO, msg);
  return nullptr;
}

static void build_worker_info_name(char *to, const char *path,
                                   const char *fname) {
  DBUG_ASSERT(to);
  char *pos = to;
  if (path[0]) pos = my_stpcpy(pos, path);
  pos = my_stpcpy(pos, "worker-");
  pos = my_stpcpy(pos, fname);
  my_stpcpy(pos, ".");
}

/**
  Initializes startup information on diferent repositories.
*/
void Rpl_info_factory::init_repository_metadata() {
  /* Needed for the file names and paths for worker info files. */
  size_t len;
  const char *relay_log_info_file_name;
  char relay_log_info_file_dirpart[FN_REFLEN];

  /* Extract the directory name from relay_log_info_file */
  dirname_part(relay_log_info_file_dirpart, relay_log_info_file, &len);
  relay_log_info_file_name = relay_log_info_file + len;

  rli_table_data.n_fields = Relay_log_info::get_number_info_rli_fields();
  rli_table_data.schema = MYSQL_SCHEMA_NAME.str;
  rli_table_data.name = RLI_INFO_NAME.str;
  rli_table_data.n_pk_fields = 0;
  rli_table_data.pk_field_indexes = nullptr;
  Relay_log_info::set_nullable_fields(&rli_table_data.nullable_fields);
  rli_file_data.n_fields = Relay_log_info::get_number_info_rli_fields();
  my_stpcpy(rli_file_data.name, relay_log_info_file);
  my_stpcpy(rli_file_data.pattern, relay_log_info_file);
  rli_file_data.name_indexed = false;
  Relay_log_info::set_nullable_fields(&rli_file_data.nullable_fields);

  mi_table_data.n_fields = Master_info::get_number_info_mi_fields();
  mi_table_data.schema = MYSQL_SCHEMA_NAME.str;
  mi_table_data.name = MI_INFO_NAME.str;
  mi_table_data.n_pk_fields = 1;
  mi_table_data.pk_field_indexes = Master_info::get_table_pk_field_indexes();
  Master_info::set_nullable_fields(&mi_table_data.nullable_fields);
  mi_file_data.n_fields = Master_info::get_number_info_mi_fields();
  my_stpcpy(mi_file_data.name, master_info_file);
  my_stpcpy(mi_file_data.pattern, master_info_file);
  mi_file_data.name_indexed = false;
  Master_info::set_nullable_fields(&mi_file_data.nullable_fields);

  worker_table_data.n_fields = Slave_worker::get_number_worker_fields();
  worker_table_data.schema = MYSQL_SCHEMA_NAME.str;
  worker_table_data.name = WORKER_INFO_NAME.str;
  worker_table_data.n_pk_fields = 2;
  worker_table_data.pk_field_indexes =
      Slave_worker::get_table_pk_field_indexes();
  Slave_worker::set_nullable_fields(&worker_table_data.nullable_fields);
  worker_file_data.n_fields = Slave_worker::get_number_worker_fields();
  build_worker_info_name(worker_file_data.name, relay_log_info_file_dirpart,
                         relay_log_info_file_name);
  build_worker_info_name(worker_file_data.pattern, relay_log_info_file_dirpart,
                         relay_log_info_file_name);
  worker_file_data.name_indexed = true;
  Slave_worker::set_nullable_fields(&worker_file_data.nullable_fields);
}

/**
  Decides during startup what repository will be used based on the following
  decision table:

  \code
  |--------------+-----------------------+-----------------------|
  | Exists \ Opt |         SOURCE        |      DESTINATION      |
  |--------------+-----------------------+-----------------------|
  | ~is_s, ~is_d |            -          | Create/Update D       |
  | ~is_s,  is_d |            -          | Continue with D       |
  |  is_s, ~is_d | Copy S into D         | Create/Update D       |
  |  is_s,  is_d | Error                 | Error                 |
  |--------------+-----------------------+-----------------------|
  \endcode

  @param[in]  info         Either master info or relay log info.
  @param[in]  option       Identifies the type of the repository that will
                           be used, i.e., destination repository.
  @param[out] handler_src  Source repository from where information is
                           copied into the destination repository.
  @param[out] handler_dest Destination repository to where informaiton is
                           copied.
  @param[out] msg          Error message if something goes wrong.

  @retval false No error
  @retval true  Failure
*/
bool Rpl_info_factory::decide_repository(Rpl_info *info, uint option,
                                         Rpl_info_handler **handler_src,
                                         Rpl_info_handler **handler_dest,
                                         const char **msg) {
  bool error = true;
  enum_return_check return_check_src = ERROR_CHECKING_REPOSITORY;
  enum_return_check return_check_dst = ERROR_CHECKING_REPOSITORY;
  DBUG_TRACE;

  if (option == INFO_REPOSITORY_DUMMY) {
    delete (*handler_src);
    *handler_src = nullptr;
    info->set_rpl_info_handler(*handler_dest);
    error = false;
    goto err;
  }

  DBUG_ASSERT((*handler_src) != nullptr && (*handler_dest) != nullptr &&
              (*handler_src) != (*handler_dest));

  return_check_src = check_src_repository(info, option, handler_src);
  return_check_dst =
      (*handler_dest)->do_check_info(info->get_internal_id());  // approx via
                                                                // scan, not
                                                                // field_values!
                                                                // Todo:
                                                                // reconsider
                                                                // any need for
                                                                // that at least
                                                                // in the Worker
                                                                // case

  if (return_check_src == ERROR_CHECKING_REPOSITORY ||
      return_check_dst == ERROR_CHECKING_REPOSITORY) {
    /*
      If there is a problem with one of the repositories we print out
      more information and exit.
    */
    return check_error_repository(*handler_src, *handler_dest, return_check_src,
                                  return_check_dst, msg);
  } else {
    if ((return_check_src == REPOSITORY_EXISTS &&
         return_check_dst == REPOSITORY_DOES_NOT_EXIST) ||
        (return_check_src == REPOSITORY_EXISTS &&
         return_check_dst == REPOSITORY_EXISTS)) {
      /*
        If there is no error, we can proceed with the normal operation.
        However, if both repositories are set an error will be printed
        out.
      */
      if (return_check_src == REPOSITORY_EXISTS &&
          return_check_dst == REPOSITORY_EXISTS) {
        *msg =
            "Multiple replication metadata repository instances "
            "found with data in them. Unable to decide which is "
            "the correct one to choose";
        goto err;
      }

      /*
        Do a low-level initialization to be able to do a state transfer.
      */
      if (init_repositories(info, handler_src, handler_dest, msg)) goto err;

      /*
        Transfer information from source to destination and delete the
        source. Note this is not fault-tolerant and a crash before removing
        source may cause the next restart to fail as is_src and is_dest may
        be true. Moreover, any failure in removing the source may lead to
        the same.
        /Alfranio
      */
      if (info->copy_info(*handler_src, *handler_dest) ||
          (*handler_dest)->flush_info(true)) {
        *msg = "Error transfering information";
        goto err;
      }
      (*handler_src)->end_info();
      if ((*handler_src)->remove_info()) {
        *msg = "Error removing old repository";
        goto err;
      }
    } else if (return_check_src == REPOSITORY_DOES_NOT_EXIST &&
               return_check_dst == REPOSITORY_EXISTS) {
      DBUG_ASSERT(info->get_rpl_info_handler() == nullptr);
      if ((*handler_dest)->do_init_info(info->get_internal_id())) {
        *msg = "Error reading repository";
        goto err;
      }
    } else {
      DBUG_ASSERT(return_check_src == REPOSITORY_DOES_NOT_EXIST &&
                  return_check_dst == REPOSITORY_DOES_NOT_EXIST);
      info->inited = false;
    }

    delete (*handler_src);
    *handler_src = nullptr;
    info->set_rpl_info_handler(*handler_dest);
    error = false;
  }

err:
  return error;
}

/**
  This method is called by the decide_repository() and is used to check if
  the source repository exits.

  @param[in]  info         Either master info or relay log info.
  @param[in]  option       Identifies the type of the repository that will
                           be used, i.e., destination repository.
  @param[out] handler_src  Source repository from where information is

  @return enum_return_check The repository's status.
*/
enum_return_check Rpl_info_factory::check_src_repository(
    Rpl_info *info, uint option, Rpl_info_handler **handler_src) {
  enum_return_check return_check_src = ERROR_CHECKING_REPOSITORY;
  bool live_migration = info->get_rpl_info_handler() != nullptr;

  if (!live_migration) {
    /*
      This is not a live migration and we don't know whether the repository
      exists or not.
    */
    return_check_src = (*handler_src)->do_check_info(info->get_internal_id());

    /*
      Since this is not a live migration, if we are using file repository
      and there is some error on table repository (for instance, engine
      disabled) we can ignore it instead of stopping replication.
      A warning saying that table is not ready to be used was logged.
    */
    if (ERROR_CHECKING_REPOSITORY == return_check_src &&
        INFO_REPOSITORY_FILE == option &&
        INFO_REPOSITORY_TABLE == (*handler_src)->do_get_rpl_info_type()) {
      return_check_src = REPOSITORY_DOES_NOT_EXIST;
      /*
        If a already existent thread was used to access info tables,
        current_thd will point to it and we must clear access error on
        it. If a temporary thread was used, then there is nothing to
        clean because the thread was already deleted.
        See Rpl_info_table_access::create_thd().
      */
      if (current_thd) current_thd->clear_error();
    }
  } else {
    /*
      This is a live migration as the repository is already associated to.
      However, we cannot assume that it really exists, for instance, if a
      file was really created.

      This situation may happen when we start a slave for the first time
      but skips its initialization and tries to migrate it.
    */
    return_check_src = (*handler_src)->do_check_info();
  }

  return return_check_src;
}

/**
  This method is called by the decide_repository() and is used print out
  information on errors.

  @param  handler_src  Source repository from where information is
                       copied into the destination repository.
  @param  handler_dest Destination repository to where informaiton is
                       copied.
  @param  err_src      Possible error status of the source repo check
  @param  err_dst      Possible error status of the destination repo check
  @param[out] msg      Error message if something goes wrong.

  @retval true  Failure
*/
bool Rpl_info_factory::check_error_repository(Rpl_info_handler *handler_src,
                                              Rpl_info_handler *handler_dest,
                                              enum_return_check err_src,
                                              enum_return_check err_dst,
                                              const char **msg) {
  bool error = true;

  /*
    If there is an error in any of the source or destination
    repository checks, the normal operation can't be proceeded.
    The runtime repository won't be initialized.
  */
  if (err_src == ERROR_CHECKING_REPOSITORY)
    LogErr(ERROR_LEVEL, ER_RPL_ERROR_CHECKING_REPOSITORY,
           handler_src->get_description_info(),
           handler_src->get_rpl_info_type_str());
  if (err_dst == ERROR_CHECKING_REPOSITORY)
    LogErr(ERROR_LEVEL, ER_RPL_ERROR_CHECKING_REPOSITORY,
           handler_dest->get_description_info(),
           handler_dest->get_rpl_info_type_str());
  *msg = "Error checking repositories";
  return error;
}

/**
  This method is called by the decide_repository() and is used to initialize
  the repositories through a low-level interfacei, which means that if they
  do not exist nothing will be created.

  @param[in]  info         Either master info or relay log info.
  @param[out] handler_src  Source repository from where information is
                           copied into the destination repository.
  @param[out] handler_dest Destination repository to where informaiton is
                           copied.
  @param[out] msg          Error message if something goes wrong.

  @retval false No error
  @retval true  Failure
*/
bool Rpl_info_factory::init_repositories(Rpl_info *info,
                                         Rpl_info_handler **handler_src,
                                         Rpl_info_handler **handler_dest,
                                         const char **msg) {
  bool live_migration = info->get_rpl_info_handler() != nullptr;

  if (!live_migration) {
    if ((*handler_src)->do_init_info(info->get_internal_id()) ||
        (*handler_dest)->do_init_info(info->get_internal_id())) {
      *msg = "Error transfering information";
      return true;
    }
  } else {
    if ((*handler_dest)->do_init_info(info->get_internal_id())) {
      *msg = "Error transfering information";
      return true;
    }
  }
  return false;
}

/**
  Creates repositories that will be associated to either the Master_info
  or Relay_log_info.

  @param[in] table_data    Defines information to create a table repository.
  @param[in] file_data     Defines information to create a file repository.
  @param[in] rep_option    Identifies the type of the repository that will
                           be used, i.e., destination repository.
  @param[out] handler_src  Source repository from where information is
                           copied into the destination repository.
  @param[out] handler_dest Destination repository to where informaiton is
                           copied.
  @param[out] msg          Error message if something goes wrong.

  @retval false No error
  @retval true  Failure
*/
bool Rpl_info_factory::init_repositories(const struct_table_data &table_data,
                                         const struct_file_data &file_data,
                                         uint rep_option,
                                         Rpl_info_handler **handler_src,
                                         Rpl_info_handler **handler_dest,
                                         const char **msg) {
  bool error = true;
  *msg = "Failed to allocate memory for master info repositories";

  DBUG_TRACE;

  DBUG_ASSERT(handler_dest != nullptr);
  switch (rep_option) {
    case INFO_REPOSITORY_FILE:
      if (!(*handler_dest = new Rpl_info_file(
                file_data.n_fields, file_data.pattern, file_data.name,
                file_data.name_indexed, &file_data.nullable_fields)))
        goto err;
      if (handler_src &&
          !(*handler_src = new Rpl_info_table(
                table_data.n_fields, table_data.schema, table_data.name,
                table_data.n_pk_fields, table_data.pk_field_indexes,
                &table_data.nullable_fields)))
        goto err;
      break;

    case INFO_REPOSITORY_TABLE:
      if (!(*handler_dest = new Rpl_info_table(
                table_data.n_fields, table_data.schema, table_data.name,
                table_data.n_pk_fields, table_data.pk_field_indexes,
                &table_data.nullable_fields)))
        goto err;
      if (handler_src &&
          !(*handler_src = new Rpl_info_file(
                file_data.n_fields, file_data.pattern, file_data.name,
                file_data.name_indexed, &file_data.nullable_fields)))
        goto err;
      break;

    case INFO_REPOSITORY_DUMMY:
      if (!(*handler_dest =
                new Rpl_info_dummy(Master_info::get_number_info_mi_fields())))
        goto err;
      break;

    default:
      DBUG_ASSERT(0);
  }
  error = false;

err:
  return error;
}

bool Rpl_info_factory::scan_repositories(uint *found_instances,
                                         uint *found_rep_option,
                                         const struct_table_data &table_data,
                                         const struct_file_data &file_data,
                                         const char **msg) {
  bool error = false;
  uint file_instances = 0;
  uint table_instances = 0;
  DBUG_ASSERT(found_rep_option != nullptr);

  DBUG_TRACE;

  if (Rpl_info_table::do_count_info(
          table_data.n_fields, table_data.schema, table_data.name,
          &table_data.nullable_fields, &table_instances)) {
    error = true;
    goto err;
  }

  if (Rpl_info_file::do_count_info(
          file_data.n_fields, file_data.pattern, file_data.name_indexed,
          &file_data.nullable_fields, &file_instances)) {
    error = true;
    goto err;
  }

  if (file_instances != 0 && table_instances != 0) {
    error = true;
    *msg =
        "Multiple repository instances found with data in "
        "them. Unable to decide which is the correct one to "
        "choose";
    goto err;
  }

  if (table_instances != 0) {
    *found_instances = table_instances;
    *found_rep_option = INFO_REPOSITORY_TABLE;
  } else if (file_instances != 0) {
    *found_instances = file_instances;
    *found_rep_option = INFO_REPOSITORY_FILE;
  } else {
    *found_instances = 0;
    *found_rep_option = INVALID_INFO_REPOSITORY;
  }

err:
  return error;
}

bool Rpl_info_factory::configure_channel_replication_filters(
    Relay_log_info *rli, const char *channel_name) {
  DBUG_TRACE;

  /*
    GROUP REPLICATION channels should not be configurable using
    --replicate* nor CHANGE REPLICATION FILTER, and should not
    inherit from global filters.
  */
  if (channel_map.is_group_replication_channel_name(channel_name)) return false;

  if (Master_info::is_configured(rli->mi)) {
    /*
      A slave replication channel would copy global replication filters
      to its per-channel replication filters if there are no per-channel
      replication filters and there are global filters on the filter type
      when it is being configured.
    */
    if (rli->rpl_filter->copy_global_replication_filters()) {
      LogErr(ERROR_LEVEL, ER_RPL_SLAVE_GLOBAL_FILTERS_COPY_FAILED,
             channel_name);
      return true;
    }
  } else {
    /*
      When starting server, users may set rpl filter options on an
      uninitialzied channel. The filter options will be reset with an
      warning.
    */
    if (!rli->rpl_filter->is_empty()) {
      LogErr(WARNING_LEVEL, ER_RPL_SLAVE_RESET_FILTER_OPTIONS, channel_name);
      rli->rpl_filter->reset();
    }
  }
  return false;
}

/**
  This function should be called from init_slave() only.

  During the server start, read all the slave repositories
  on disk (either in FILE or TABLE form) and create corresponding
  slave info objects. Each thus created master_info object is
  added to pchannel_map.

  Multisource replication is supported by only TABLE based
  repositories. Based on this fact, the following table shows
  the supported cases considering the repository type and
  multiple channels of a slave.
  Each <---> represents a channel with a name on top of it.
  '' is an empty stringed channel (or default channel).
  'N' indicates some name for a channel.

 +-----------------------------+------------------+-----------+
 | channels                    | Supported? FILE  |  TABLE    |
 +-----------------------------+------------------+-----------+
 |              ''             |                  |           |
 | A)  Master<------->Slave    |  YES             |  YES      |
 |                             |                  |           |
 |                             |                  |           |
 |              'N'            |                  |           |
 | B) Master<------->Slave     |  NO              |  YES      |
 |                             |                  |           |
 |              ''             |                  |           |
 |    Master0<------------+    |                  |           |
 |              'N'       v    |  NO              |  YES      |
 | C) Master1<----------->Slave|                  |           |
 |              'N'       ^    |                  |           |
 |    Mastern<------------+    |                  |           |
 |                             |                  |           |
 |                             |                  |           |
 |              'N'            |                  |           |
 |    Master1<------------+    |                  |           |
 |              'N'       v    |   NO             |   YES     |
 | D) Master2<----------->Slave|                  |           |
 |              'N'       ^    |                  |           |
 |    Mastern<------------+    |                  |           |
 |                             |                  |           |
 |                             |                  |           |
 +-----------------------------+------------------+-----------+

 In a new server, A) shown above is created by default.
 If there are multiple 'named' channels, but and if a default_channel
 is not created, it is created.

 Some points to note from the above table
 =========================================

 From the table it also follows that conversion of repositories
 is possible *ONLY* in the case of A) i.e for ex: if B) type repository
 (i.e a named slave channel) was found during server starup but the user
 repository option is INFO_REPOSITORY_FILE, then we exit the function.

 @note: only for type A) i.e default channel, it is permissable to
        have different repo types for Master_info and Relay_log_info
        (Ex: FILE for mi and TABLE for rli)

 @note: The above restrictions break factory pattern in the code
        which has been followed throughout before.

 @note: All the repository conversion(or live migration) functions
       (ex: decide_repository()) take Rpl_info::internal_id as an
       identifier which is always 1 for the case of Master_info and
       Relay_log_info. So, in the case of multisource replication,
       the decision to convert the repositories shall be made even before
       invoking decide_repository(). In other words, if a channel is not a
       default channel('') we shall not invoke decide_repository().

 @note:  In general, the algorithm in creation of slave info object is:
          l1: new slave_info;
          l2: Initialize the repository handlers
          l3: if (default_channel)
                 check and convert repositories
              else
                   // TABLE type repository
                  set the value of PK in the TABLE handler.

 @note: Update from 5.6 to 5.7(which has Channel_Name in slave_info_tables)
        is handled in the upgrade script as usual.


  @param[in]       mi_option         the user provided master_info_repository
  @param[in]       rli_option        the user provided relay_log_info_repository
  @param[in]       thread_mask       thread mask
  @param[in]       pchannel_map          the pointer to the multi source map
                                     (see, rpl_msr.h)

  @retval          false             success
  @retval          true              fail
*/

bool Rpl_info_factory::create_slave_info_objects(
    uint mi_option, uint rli_option, int thread_mask,
    Multisource_info *pchannel_map) {
  DBUG_TRACE;

  Master_info *mi = nullptr;
  const char *msg = nullptr;
  bool error = false, channel_error;
  bool default_channel_existed_previously = false;

  std::vector<std::string> channel_list;

  /* Number of instances of Master_info repository */
  uint mi_instances = 0;

  /* At this point, the repository in invalid or unknown */
  uint mi_repository = INVALID_INFO_REPOSITORY;

  /*
    Number of instances of Relay_log_info_repository.
    (Number of Slave worker objects that will be created by the Coordinator
    (when slave_parallel_workers>0) at a later stage and not here).
  */
  uint rli_instances = 0;

  /* At this point, the repository is invalid or unknown */
  uint rli_repository = INVALID_INFO_REPOSITORY;

  /*
    Initialize the repository metadata. This metadata is the
    name of files to look in case of FILE type repository, and the
    names of table to look in case of TABLE type repository.
  */
  Rpl_info_factory::init_repository_metadata();

  /* Count the number of Master_info and Relay_log_info repositories */
  if (scan_repositories(&mi_instances, &mi_repository, mi_table_data,
                        mi_file_data, &msg) ||
      scan_repositories(&rli_instances, &rli_repository, rli_table_data,
                        rli_file_data, &msg)) {
    /* msg will contain the reason of failure */
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_GENERIC_MESSAGE, msg);
    error = true;
    goto end;
  }

  /* Make a list of all channels if the slave was connected to previously*/
  if (load_channel_names_from_repository(channel_list, mi_instances,
                                         mi_repository,
                                         pchannel_map->get_default_channel(),
                                         &default_channel_existed_previously)) {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_COULD_NOT_CREATE_CHANNEL_LIST);
    error = true;
    goto end;
  }

  if ((mi_option == INFO_REPOSITORY_FILE ||
       rli_option == INFO_REPOSITORY_FILE) &&
      channel_list.size() > 1) {
    /* Not supported cases of B) C) and D) above */
    LogErr(ERROR_LEVEL, ER_RPL_MULTISOURCE_REQUIRES_TABLE_TYPE_REPOSITORIES);
    error = true;
    goto end;
  }

  /* Adding the default channel if needed. */
  if (!default_channel_existed_previously) {
    std::string str(pchannel_map->get_default_channel());
    channel_list.push_back(str);
  }

  /*
    Create and initialize the channels.

    Even if there is an error during one channel creation, we continue to
    iterate until we have created the other channels.

    For compatibility reasons, we have to separate the print out
    of the error messages.
  */
  for (std::vector<std::string>::iterator it = channel_list.begin();
       it != channel_list.end(); ++it) {
    const char *cname = (*it).c_str();
    bool is_default_channel =
        !strcmp(cname, pchannel_map->get_default_channel());
    channel_error =
        !(mi = create_mi_and_rli_objects(
              mi_option, rli_option, cname,
              (channel_list.size() == 1) ? true : false, pchannel_map));
    /*
      Read the channel configuration from the repository if the channel name
      was read from the repository.
    */
    if (!channel_error &&
        (!is_default_channel || default_channel_existed_previously)) {
      bool ignore_if_no_info = (channel_list.size() == 1) ? true : false;
      channel_error =
          load_mi_and_rli_from_repositories(mi, ignore_if_no_info, thread_mask);
    }

    if (!channel_error) {
      error = configure_channel_replication_filters(mi->rli, cname);
    } else {
      LogErr(ERROR_LEVEL, ER_RPL_SLAVE_FAILED_TO_INIT_A_MASTER_INFO_STRUCTURE,
             cname);
    }
    error = error || channel_error;
  }
end:
  return error;
}

/**
   Create Master_info and Relay_log_info objects for a new channel.
   Also, set cross dependencies between these objects used all over
   the code.

   Both master_info and relay_log_info repositories should be of the type
   TABLE. We do a check for this here as well.

   @param[in]    mi_option        master info repository
   @param[in]    rli_option       relay log info repository
   @param[in]    channel          the channel for which these objects
                                  should be created.
   @param[in]    to_decide_repo   For this channel, check if repositories
                                  are allowed to convert from one type to other.
   @param[in]    pchannel_map     a pointer to channel_map

   @return      Pointer         pointer to the created Master_info
   @return      NULL            when creation fails

*/

Master_info *Rpl_info_factory::create_mi_and_rli_objects(
    uint mi_option, uint rli_option, const char *channel, bool to_decide_repo,
    Multisource_info *pchannel_map) {
  DBUG_TRACE;

  Master_info *mi = nullptr;
  Relay_log_info *rli = nullptr;

  if (!(mi = Rpl_info_factory::create_mi(mi_option, channel, to_decide_repo)))
    return nullptr;

  if (!(rli = Rpl_info_factory::create_rli(rli_option, relay_log_recovery,
                                           channel, to_decide_repo))) {
    mi->channel_wrlock();
    delete mi;
    mi = nullptr;
    return nullptr;
  }

  /* Set the cross dependencies used all over the code */
  mi->set_relay_log_info(rli);
  rli->set_master_info(mi);

  /* Add to multisource map*/
  if (pchannel_map->add_mi(channel, mi)) {
    mi->channel_wrlock();
    delete mi;
    delete rli;
    return nullptr;
  }

  return mi;
}

/**
   Make a list of all the channels if existed on the previos slave run.

   @param[out]  channel_list    the names of all channels that exists
                                on this slave.

   @param[in]   mi_instances    number of master_info repositories

   @param[in]   mi_repository   Found master_info repository

   @param[in]   default_channel pointer to default channel.

   @param[out]  default_channel_existed_previously
                                Value filled with true if default channel
                                existed previously. False if it is not.

   @retval      true            fail
   @retval      false           success

*/

bool Rpl_info_factory::load_channel_names_from_repository(
    std::vector<std::string> &channel_list,
    uint mi_instances MY_ATTRIBUTE((unused)), uint mi_repository,
    const char *default_channel, bool *default_channel_existed_previously) {
  DBUG_TRACE;

  *default_channel_existed_previously = false;
  switch (mi_repository) {
    case INFO_REPOSITORY_FILE:
      DBUG_ASSERT(mi_instances == 1);
      /* insert default channel */
      {
        std::string str(default_channel);
        channel_list.push_back(str);
      }
      *default_channel_existed_previously = true;
      break;
    case INFO_REPOSITORY_TABLE:
      if (load_channel_names_from_table(channel_list, default_channel,
                                        default_channel_existed_previously))
        return true;
      break;
    case INVALID_INFO_REPOSITORY:
      /* file and table instanaces are zero, nothing to be done*/
      break;
    default:
      DBUG_ASSERT(0);
  }

  return false;
}

/**
  In a multisourced slave, during init_slave(), the repositories
  are read to initialize the slave info objects. To initialize
  the slave info objects, we need the number of channels the slave
  was connected to previously. The following function, finds the
  number of channels in the master info repository.
  Later, this chanenl list is used to create a pair of {mi, rli}
  objects required for IO and SQL threads respectively.

  @param [out]  channel_list    A reference to the channel list.
                                This will be filled after reading the
                                master info table, row by row.

  @param[in]   default_channel  pointer to default channel.

  @param[out]  default_channel_existed_previously
                                Value filled with true if default channel
                                existed previously. False if it is not.

  @todo: Move it to Rpl_info_table and make it generic to extract
         all the PK list from the tables (but it not yet necessary)
*/
bool Rpl_info_factory::load_channel_names_from_table(
    std::vector<std::string> &channel_list, const char *default_channel,
    bool *default_channel_existed_previously) {
  DBUG_TRACE;

  int error = 1;
  TABLE *table = nullptr;
  ulong saved_mode;
  Open_tables_backup backup;
  Rpl_info_table *info = nullptr;
  THD *thd = nullptr;
  char buff[MAX_FIELD_WIDTH];
  *default_channel_existed_previously = false;
  String str(buff, sizeof(buff),
             system_charset_info);  // to extract channel names

  uint channel_field = Master_info::get_channel_field_num() - 1;

  if (!(info = new Rpl_info_table(mi_table_data.n_fields, mi_table_data.schema,
                                  mi_table_data.name, mi_table_data.n_pk_fields,
                                  mi_table_data.pk_field_indexes,
                                  &mi_table_data.nullable_fields)))
    return true;

  thd = info->access->create_thd();
  saved_mode = thd->variables.sql_mode;

  /*
     Opens and locks the rpl_info table before accessing it.
  */
  if (info->access->open_table(thd, to_lex_cstring(info->str_schema),
                               to_lex_cstring(info->str_table),
                               info->get_number_info(), TL_READ, &table,
                               &backup)) {
    /*
      We cannot simply print out a warning message at this
      point because this may represent a bootstrap.
    */
    error = 0;
    goto err;
  }

  /* Do ha_handler random init for full scanning */
  if ((error = table->file->ha_rnd_init(true))) return true;

  /* Ensure that the table pk (Channel_name) is at the correct position */
  if (info->verify_table_primary_key_fields(table)) {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_FAILED_TO_CREATE_CHANNEL_FROM_MASTER_INFO);
    error = -1;
    goto err;
  }

  /*
    Load all the values in record[0] for each row
    and then extract channel name from it
  */

  do {
    error = table->file->ha_rnd_next(table->record[0]);
    switch (error) {
      case 0:
        /* extract the channel name from table->field and append to the list */
        table->field[channel_field]->val_str(&str);
        channel_list.push_back(std::string(str.c_ptr_safe()));
        if (!strcmp(str.c_ptr_safe(), default_channel))
          *default_channel_existed_previously = true;
        break;

      case HA_ERR_END_OF_FILE:
        break;

      default:
        DBUG_PRINT("info", ("Failed to get next record"
                            " (ha_rnd_next returns %d)",
                            error));
    }
  } while (!error);

  /*close the table */
err:

  table->file->ha_rnd_end();
  info->access->close_table(thd, table, &backup, error);
  thd->variables.sql_mode = saved_mode;
  info->access->drop_thd(thd);
  delete info;
  return error != HA_ERR_END_OF_FILE && error != 0;
}
