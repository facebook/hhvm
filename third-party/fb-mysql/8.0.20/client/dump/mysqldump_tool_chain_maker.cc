/*
  Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "client/dump/mysqldump_tool_chain_maker.h"

#include <stddef.h>
#include <boost/algorithm/string.hpp>
#include <functional>

#include "client/dump/compression_lz4_writer.h"
#include "client/dump/compression_zlib_writer.h"
#include "client/dump/file_writer.h"
#include "client/dump/i_output_writer.h"
#include "client/dump/mysqldump_tool_chain_maker_options.h"
#include "client/dump/sql_formatter.h"
#include "client/dump/standard_writer.h"
#include "client/dump/view.h"
#include "m_ctype.h"

using namespace Mysql::Tools::Dump;
using std::placeholders::_1;

void Mysqldump_tool_chain_maker::delete_chain(uint64, I_object_reader *) {}

I_object_reader *Mysqldump_tool_chain_maker::create_chain(
    Chain_data *, I_dump_task *dump_task) {
  Table_rows_dump_task *rows_task =
      dynamic_cast<Table_rows_dump_task *>(dump_task);
  if (rows_task != nullptr &&
      (m_options->m_skip_rows_data ||
       rows_task->get_related_table()->get_type() == "FEDERATED" ||
       rows_task->get_related_table()->get_type() == "MRG_ISAM" ||
       !this->compare_no_case_latin_with_db_string(
           "MRG_MyISAM", rows_task->get_related_table()->get_type()))) {
    return nullptr;
  }

  Abstract_data_object *object =
      dynamic_cast<Abstract_data_object *>(dump_task->get_related_db_object());
  if (!m_options->is_object_included_in_dump(object)) {
    return nullptr;
  }
  /*
    View dependency check is moved post filteration. This will ensure that
    only filtered out views will be checked for their dependecies. This
    allows mysqlpump to dump a database even when there exsits an invalid
    view in another database which user is not interested to dump. I_S views
    are skipped from this check.
  */
  if (object && (dynamic_cast<View *>(object) != nullptr) &&
      my_strcasecmp(&my_charset_latin1, object->get_schema().c_str(),
                    INFORMATION_SCHEMA_DB_NAME)) {
    Mysql::Tools::Base::Mysql_query_runner *runner = this->get_runner();
    /* Check if view dependent objects exists */
    if (runner->run_query(std::string("LOCK TABLES ") +
                          this->get_quoted_object_full_name(
                              object->get_schema(), object->get_name()) +
                          " READ") != 0)
      return nullptr;
    else
      runner->run_query(std::string("UNLOCK TABLES"));
    delete runner;
  }

  if (m_main_object_reader == nullptr) {
    I_output_writer *writer;
    if (m_options->m_result_file.has_value())
      writer = new File_writer(this->get_message_handler(),
                               this->get_object_id_generator(),
                               m_options->m_result_file.value());
    else
      writer = new Standard_writer(this->get_message_handler(),
                                   this->get_object_id_generator());
    if (writer->init()) {
      delete writer;
      return nullptr;
    }
    m_all_created_elements.push_back(writer);
    if (m_options->m_compress_output_algorithm.has_value()) {
      std::string algorithm_name =
          m_options->m_compress_output_algorithm.value();
      boost::to_lower(algorithm_name);

      Abstract_output_writer_wrapper *compression_writer_as_wrapper = nullptr;
      I_output_writer *compression_writer_as_writer = nullptr;
      if (algorithm_name == "lz4") {
        Compression_lz4_writer *compression_writer = new Compression_lz4_writer(
            this->get_message_handler(), this->get_object_id_generator());
        if (compression_writer->init()) {
          delete compression_writer;
          return nullptr;
        }
        compression_writer_as_wrapper = compression_writer;
        compression_writer_as_writer = compression_writer;
      } else if (algorithm_name == "zlib") {
        Compression_zlib_writer *compression_writer =
            new Compression_zlib_writer(this->get_message_handler(),
                                        this->get_object_id_generator(),
                                        Z_DEFAULT_COMPRESSION);
        if (compression_writer->init()) {
          delete compression_writer;
          return nullptr;
        }
        compression_writer_as_wrapper = compression_writer;
        compression_writer_as_writer = compression_writer;
      } else {
        this->pass_message(Mysql::Tools::Base::Message_data(
            0, "Unknown compression method: " + algorithm_name,
            Mysql::Tools::Base::Message_type_error));
        return nullptr;
      }
      compression_writer_as_wrapper->register_output_writer(writer);
      writer = compression_writer_as_writer;
      m_all_created_elements.push_back(writer);
    }
    Sql_formatter *formatter = new Sql_formatter(
        this->get_connection_provider(), this->get_message_handler(),
        this->get_object_id_generator(), m_options,
        m_options->m_formatter_options);
    this->register_progress_watchers_in_child(formatter);
    formatter->register_output_writer(writer);
    m_all_created_elements.push_back(formatter);

    m_main_object_reader = new Mysql_object_reader(
        this->get_connection_provider(), this->get_message_handler(),
        this->get_object_id_generator(), m_options->m_object_reader_options);
    this->register_progress_watchers_in_child(m_main_object_reader);
    m_main_object_reader->register_data_formatter(formatter);
    m_all_created_elements.push_back(m_main_object_reader);
  }
  /*
    run as a single process only if default parallelism is set to 0 and
    parallel schemas is not set
  */
  if (m_options->m_default_parallelism == 0 &&
      m_options->get_parallel_schemas_thread_count() == 0) {
    return m_main_object_reader;
  }
  Abstract_data_object *data_object =
      dynamic_cast<Abstract_data_object *>(dump_task->get_related_db_object());

  int object_queue_id = (data_object != nullptr)
                            ? (m_options->get_object_queue_id_for_schema(
                                  data_object->get_schema()))
                            : 0;
  std::map<int, Object_queue *>::iterator it =
      m_object_queues.find(object_queue_id);
  if (it != m_object_queues.end()) {
    return it->second;
  }
  Object_queue *queue = new Object_queue(
      this->get_message_handler(), this->get_object_id_generator(),
      m_options->get_object_queue_threads_count(object_queue_id),
      new std::function<void(bool)>(std::bind(
          &Mysqldump_tool_chain_maker::mysql_thread_callback, this, _1)),
      m_program);
  this->register_progress_watchers_in_child(queue);
  queue->register_object_reader(m_main_object_reader);
  m_all_created_elements.push_back(queue);
  m_object_queues.insert(std::make_pair(object_queue_id, queue));
  return queue;
}

void Mysqldump_tool_chain_maker::stop_queues() {
  std::map<int, Object_queue *>::const_iterator iter;
  for (iter = m_object_queues.begin(); iter != m_object_queues.end(); iter++) {
    iter->second->stop_queue();
  }
}

void Mysqldump_tool_chain_maker::mysql_thread_callback(bool is_starting) {
  if (is_starting)
    mysql_thread_init();
  else
    mysql_thread_end();
}

Mysqldump_tool_chain_maker::~Mysqldump_tool_chain_maker() {
  for (std::vector<I_chain_element *>::reverse_iterator it =
           m_all_created_elements.rbegin();
       it != m_all_created_elements.rend(); ++it) {
    delete *it;
  }
}

Mysqldump_tool_chain_maker::Mysqldump_tool_chain_maker(
    I_connection_provider *connection_provider,
    std::function<bool(const Mysql::Tools::Base::Message_data &)>
        *message_handler,
    Simple_id_generator *object_id_generator,
    Mysqldump_tool_chain_maker_options *options,
    Mysql::Tools::Base::Abstract_program *program)
    : Abstract_chain_element(message_handler, object_id_generator),
      Abstract_mysql_chain_element_extension(
          connection_provider, message_handler,
          options->m_mysql_chain_element_options),
      m_options(options),
      m_main_object_reader(nullptr),
      m_program(program) {}
