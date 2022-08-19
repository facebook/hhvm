/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_INFO_FACTORY_H
#define RPL_INFO_FACTORY_H

#include <sys/types.h>
#include <string>
#include <vector>

#include "my_bitmap.h"
#include "my_io.h"
#include "sql/rpl_info_handler.h"  // enum_return_check

class Master_info;
class Multisource_info;
class Relay_log_info;
class Rpl_info;
class Slave_worker;

extern ulong opt_mi_repository_id;
extern ulong opt_rli_repository_id;

class Rpl_info_factory {
 public:
  static bool create_slave_info_objects(uint mi_option, uint rli_option,
                                        int thread_mask,
                                        Multisource_info *pchannel_map);

  /**
    Establish the relation between the channel's replication filters and
    the channel's Relay_log_info, and copy global replication filters to
    the channel's replication filters if needed.

    @param rli Pointer to Relay_log_info.
    @param channel_name The channel name.

    @retval false No error
    @retval true  Failure
  */
  static bool configure_channel_replication_filters(Relay_log_info *rli,
                                                    const char *channel_name);

  static Master_info *create_mi_and_rli_objects(uint mi_option, uint rli_option,
                                                const char *channel,
                                                bool convert_repo,
                                                Multisource_info *channel_map);

  static Master_info *create_mi(uint rli_option, const char *channel,
                                bool conver_repo);
  static bool change_mi_repository(Master_info *mi, const uint mi_option,
                                   const char **msg);
  static Relay_log_info *create_rli(uint rli_option, bool is_slave_recovery,
                                    const char *channel, bool convert_repo);
  static bool change_rli_repository(Relay_log_info *rli, const uint rli_option,
                                    const char **msg);
  static Slave_worker *create_worker(uint rli_option, uint worker_id,
                                     Relay_log_info *rli,
                                     bool is_gaps_collecting_phase);
  static bool reset_workers(Relay_log_info *rli);

 private:
  typedef struct file_data {
    uint n_fields;
    char name[FN_REFLEN];
    char pattern[FN_REFLEN];
    bool name_indexed;  // whether file name should include instance number
    MY_BITMAP nullable_fields;
    virtual ~file_data() { bitmap_free(&nullable_fields); }
  } struct_file_data;

  typedef struct table_data {
    uint n_fields;
    const char *schema;
    const char *name;
    uint n_pk_fields;
    const uint *pk_field_indexes;
    MY_BITMAP nullable_fields;
    virtual ~table_data() { bitmap_free(&nullable_fields); }
  } struct_table_data;

  static struct_table_data rli_table_data;
  static struct_file_data rli_file_data;
  static struct_table_data mi_table_data;
  static struct_file_data mi_file_data;
  static struct_table_data worker_table_data;
  static struct_file_data worker_file_data;

  static void init_repository_metadata();
  static bool decide_repository(Rpl_info *info, uint option,
                                Rpl_info_handler **handler_src,
                                Rpl_info_handler **handler_dest,
                                const char **msg);
  static bool init_repositories(const struct_table_data &table_data,
                                const struct_file_data &file_data, uint option,
                                Rpl_info_handler **handler_src,
                                Rpl_info_handler **handler_dest,
                                const char **msg);

  static enum_return_check check_src_repository(Rpl_info *info, uint option,
                                                Rpl_info_handler **handler_src);
  static bool check_error_repository(Rpl_info_handler *handler_src,
                                     Rpl_info_handler *handler_dst,
                                     enum_return_check err_src,
                                     enum_return_check err_dst,
                                     const char **msg);
  static bool init_repositories(Rpl_info *info, Rpl_info_handler **handler_src,
                                Rpl_info_handler **handler_dst,
                                const char **msg);
  static bool scan_repositories(uint *found_instances, uint *found_rep_option,
                                const struct_table_data &table_data,
                                const struct_file_data &file_data,
                                const char **msg);
  static bool load_channel_names_from_repository(
      std::vector<std::string> &channel_list, uint mi_instances,
      uint mi_repository, const char *default_channel,
      bool *default_channel_created_previously);

  static bool load_channel_names_from_table(
      std::vector<std::string> &channel_list, const char *default_channel,
      bool *default_channel_created_previously);
};

#endif
