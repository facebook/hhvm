/*
   Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef BINLOG_TOOLS_ITERATORS_H
#define BINLOG_TOOLS_ITERATORS_H

#include <queue>
#include "sql/binlog_reader.h"

namespace binlog {
namespace tools {

class Iterator {
 protected:
  std::queue<Log_event *> m_events;
  Binlog_file_reader *m_binlog_reader;
  bool m_verify_checksum;
  bool m_copy_event_buffer;
  int m_error_number;
  std::string m_error_message;

 public:
  Iterator(Binlog_file_reader *);
  virtual ~Iterator();

  virtual void unset_copy_event_buffer();
  virtual void set_copy_event_buffer();

  virtual void unset_verify_checksum();
  virtual void set_verify_checksum();

  virtual bool has_error();
  virtual int get_error_number();
  virtual std::string get_error_message();

  virtual Log_event *begin();
  virtual Log_event *end();
  virtual Log_event *next();

 protected:
  virtual Log_event *do_next();
};

}  // namespace tools
}  // namespace binlog

#endif
