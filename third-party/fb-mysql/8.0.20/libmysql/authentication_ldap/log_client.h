/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <iostream>
#include <sstream>

struct ldap_log_type {
  typedef enum {
    LDAP_LOG_DBG,
    LDAP_LOG_INFO,
    LDAP_LOG_WARNING,
    LDAP_LOG_ERROR
  } ldap_type;
};

enum ldap_log_level {
  LDAP_LOG_LEVEL_NONE = 1,
  LDAP_LOG_LEVEL_ERROR,
  LDAP_LOG_LEVEL_ERROR_WARNING,
  LDAP_LOG_LEVEL_ERROR_WARNING_INFO,
  LDAP_LOG_LEVEL_ALL
};

class Ldap_log_writer_error {
 public:
  Ldap_log_writer_error();
  ~Ldap_log_writer_error();
  void write(std::string data);
};

class Ldap_logger {
 public:
  Ldap_logger();
  ~Ldap_logger();
  template <ldap_log_type::ldap_type type>
  void log(std::string msg);
  void set_log_level(ldap_log_level level);

 private:
  Ldap_log_writer_error *m_log_writer;
  ldap_log_level m_log_level;
};

template <ldap_log_type::ldap_type type>
void Ldap_logger::log(std::string msg) {
  std::stringstream log_stream;
  switch (type) {
    case ldap_log_type::LDAP_LOG_DBG:
      if (LDAP_LOG_LEVEL_ALL > m_log_level) {
        return;
      }
      log_stream << "[DBG] ";
      break;
    case ldap_log_type::LDAP_LOG_INFO:
      if (LDAP_LOG_LEVEL_ERROR_WARNING_INFO > m_log_level) {
        return;
      }
      log_stream << "[Note] ";
      break;
    case ldap_log_type::LDAP_LOG_WARNING:
      if (LDAP_LOG_LEVEL_ERROR_WARNING > m_log_level) {
        return;
      }
      log_stream << "[Warning] ";
      break;
    case ldap_log_type::LDAP_LOG_ERROR:
      if (LDAP_LOG_LEVEL_NONE >= m_log_level) {
        return;
      }
      log_stream << "[Error] ";
      break;
  };

  /** We can write debug messages also in error log file if logging level is set
     to debug. For MySQL client this will come from environment variable */
  if (m_log_writer) {
    log_stream << ": " << msg;
    m_log_writer->write(log_stream.str());
  }
}

extern Ldap_logger *g_logger_client;

#define log_dbg g_logger_client->log<ldap_log_type::LDAP_LOG_DBG>
#define log_info g_logger_client->log<ldap_log_type::LDAP_LOG_INFO>
#define log_warning g_logger->log<ldap_log_type::LDAP_LOG_WARNING>
#define log_error g_logger_client->log<ldap_log_type::LDAP_LOG_ERROR>

#endif
