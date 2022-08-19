/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

/**
  @addtogroup Backup
  @{

  @file

  @brief Log resource definitions. This includes code for the server
         resources that will take part on the results of the
         performance_schema.log_status table.
*/

#ifndef LOG_RESOURCE_H
#define LOG_RESOURCE_H

#include "sql/binlog.h"
#include "sql/handler.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_mi.h"

class Json_dom;

/**
  @class Log_resource

  This is the base class that the logic of collecting a MySQL server instance
  resources log will call.

  It basically contains lock, unlock and collect functions that shall be
  overridden by more specialized classes to handle the specific cases of
  resources participating in the process.
*/

class Log_resource {
  /* JSON object to be filled by the do_collect_info function */
  Json_dom *json = nullptr;

 public:
  /**
    There must be one function of this kind in order for the symbols in the
    server's dynamic library to be visible to plugins.
  */

  static int dummy_function_to_ensure_we_are_linked_into_the_server();

  /**
    Log_resource constructor.

    @param[in] json_arg the pointer to the JSON object to be populated with the
                        resource log information.
  */

  Log_resource(Json_dom *json_arg) : json(json_arg) {}

  /**
    Destructor.

    This function will point the JSON object to nullptr.
  */

  virtual ~Log_resource() { json = nullptr; }

  /**
    Return the pointer to the JSON object that should be used to fill the
    resource log information.

    @return  the Json_object pointer to be filled with the log information.
  */

  Json_dom *get_json() { return json; }

  /*
    Next three virtual functions need to be overridden by any more specialized
    class inheriting from this to support a specific resource participating in
    the collection process.
  */

  /**
    Lock the resource avoiding updates.
  */

  virtual void lock() = 0;

  /**
    Unlock the resource allowing updates.
  */

  virtual void unlock() = 0;

  /**
    Collect resource log information.

    @return  false on success.
             true if there was an error collecting the information.
  */

  virtual bool collect_info() = 0;
};

/**
  @class Log_resource_mi_wrapper

  This is the Log_resource to handle Master_info resources.
*/
class Log_resource_mi_wrapper : public Log_resource {
  Master_info *mi = nullptr;

 public:
  /**
    Log_resource_mi_wrapper constructor.

    @param[in] mi_arg the pointer to the Master_info object resource.
    @param[in] json_arg the pointer to the JSON object to be populated with the
                        resource log information.
  */

  Log_resource_mi_wrapper(Master_info *mi_arg, Json_dom *json_arg)
      : Log_resource(json_arg), mi(mi_arg) {}

  void lock() override;
  void unlock() override;
  bool collect_info() override;
};

/**
  @class Log_resource_binlog_wrapper

  This is the Log_resource to handle MYSQL_BIN_LOG resources.
*/
class Log_resource_binlog_wrapper : public Log_resource {
  MYSQL_BIN_LOG *binlog = nullptr;

 public:
  /**
    Log_resource_binlog_wrapper constructor.

    @param[in] binlog_arg the pointer to the MYSQL_BIN_LOG object resource.
    @param[in] json_arg the pointer to the JSON object to be populated with the
                        resource log information.
  */

  Log_resource_binlog_wrapper(MYSQL_BIN_LOG *binlog_arg, Json_dom *json_arg)
      : Log_resource(json_arg), binlog(binlog_arg) {}

  void lock() override;
  void unlock() override;
  bool collect_info() override;
};

/**
  @class Log_resource_gtid_state_wrapper

  This is the Log_resource to handle Gtid_state resources.
*/
class Log_resource_gtid_state_wrapper : public Log_resource {
  Gtid_state *gtid_state = nullptr;

 public:
  /**
    Log_resource_gtid_state_wrapper constructor.

    @param[in] gtid_state_arg the pointer to the Gtid_state object resource.
    @param[in] json_arg the pointer to the JSON object to be populated with the
                        resource log information.
  */

  Log_resource_gtid_state_wrapper(Gtid_state *gtid_state_arg,
                                  Json_dom *json_arg)
      : Log_resource(json_arg), gtid_state(gtid_state_arg) {}

  void lock() override;
  void unlock() override;
  bool collect_info() override;
};

/**
  @class Log_resource_hton_wrapper

  This is the Log_resource to handle handlerton resources.
*/
class Log_resource_hton_wrapper : public Log_resource {
  handlerton *hton = nullptr;

 public:
  /**
    Log_resource_hton_wrapper constructor.

    @param[in] hton_arg the pointer to the hton resource.
    @param[in] json_arg the pointer to the JSON object to be populated with the
                        resource log information.
  */

  Log_resource_hton_wrapper(handlerton *hton_arg, Json_dom *json_arg)
      : Log_resource(json_arg), hton(hton_arg) {}

  void lock() override;
  void unlock() override;
  bool collect_info() override;
};

/**
  @class Log_resource_factory

  This is the Log_resource factory to create wrappers for supported
  resources.
*/
class Log_resource_factory {
 public:
  /**
    Creates a Log_resource wrapper based on a Master_info object.

    @param[in] mi the pointer to the Master_info object resource.
    @param[in] json the pointer to the JSON object to be populated with the
                    resource log information.
    @return  the pointer to the new Log_resource.
  */

  static Log_resource *get_wrapper(Master_info *mi, Json_dom *json);

  /**
    Creates a Log_resource wrapper based on a Master_info object.

    @param[in] binlog the pointer to the MYSQL_BIN_LOG object resource.
    @param[in] json the pointer to the JSON object to be populated with the
                    resource log information.
    @return  the pointer to the new Log_resource.
  */

  static Log_resource *get_wrapper(MYSQL_BIN_LOG *binlog, Json_dom *json);

  /**
    Creates a Log_resource wrapper based on a Gtid_state object.

    @param[in] gtid_state the pointer to the Gtid_state object resource.
    @param[in] json the pointer to the JSON object to be populated with the
                    resource log information.
    @return  the pointer to the new Log_resource.
  */

  static Log_resource *get_wrapper(Gtid_state *gtid_state, Json_dom *json);

  /**
    Creates a Log_resource wrapper based on a handlerton.

    @param[in] hton the pointer to the handlerton resource.
    @param[in] json the pointer to the JSON object to be populated with the
                    resource log information.
    @return  the pointer to the new Log_resource.
  */

  static Log_resource *get_wrapper(handlerton *hton, Json_dom *json);
};

/**
  @} (End of group Backup)
*/

#endif /* LOG_RESOURCE_H */
