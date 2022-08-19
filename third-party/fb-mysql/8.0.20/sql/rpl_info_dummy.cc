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

#include "sql/rpl_info_dummy.h"

#include <stddef.h>

#include "my_compiler.h"
#include "my_dbug.h"

class Server_ids;

Rpl_info_dummy::Rpl_info_dummy(const int nparam)
    : Rpl_info_handler(nparam, nullptr) {}

int Rpl_info_dummy::do_init_info(uint instance MY_ATTRIBUTE((unused))) {
  return 0;
}

int Rpl_info_dummy::do_init_info() { return 0; }

int Rpl_info_dummy::do_prepare_info_for_read() {
  DBUG_ASSERT(!abort);
  cursor = 0;
  return 0;
}

int Rpl_info_dummy::do_prepare_info_for_write() {
  DBUG_ASSERT(!abort);
  cursor = 0;
  return 0;
}

enum_return_check Rpl_info_dummy::do_check_info() {
  DBUG_ASSERT(!abort);
  return REPOSITORY_DOES_NOT_EXIST;
}

enum_return_check Rpl_info_dummy::do_check_info(
    uint instance MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);
  return REPOSITORY_DOES_NOT_EXIST;
}

int Rpl_info_dummy::do_flush_info(const bool force MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);
  return 0;
}

void Rpl_info_dummy::do_end_info() { return; }

int Rpl_info_dummy::do_remove_info() {
  DBUG_ASSERT(!abort);
  return 0;
}

int Rpl_info_dummy::do_clean_info() {
  DBUG_ASSERT(!abort);
  return 0;
}

uint Rpl_info_dummy::do_get_rpl_info_type() { return INFO_REPOSITORY_DUMMY; }

bool Rpl_info_dummy::do_set_info(const char *format MY_ATTRIBUTE((unused)),
                                 va_list args MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_set_info(const int pos MY_ATTRIBUTE((unused)),
                                 const char *value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_set_info(const int pos MY_ATTRIBUTE((unused)),
                                 const uchar *value MY_ATTRIBUTE((unused)),
                                 const size_t size MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_set_info(const int pos MY_ATTRIBUTE((unused)),
                                 const ulong value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_set_info(const int pos MY_ATTRIBUTE((unused)),
                                 const int value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_set_info(const int pos MY_ATTRIBUTE((unused)),
                                 const float value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_set_info(const int pos MY_ATTRIBUTE((unused)),
                                 const Server_ids *value
                                     MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_set_info(const int, const std::nullptr_t) {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_set_info(const int, const std::nullptr_t,
                                 const size_t) {
  DBUG_ASSERT(!abort);

  return false;
}

Rpl_info_handler::enum_field_get_status Rpl_info_dummy::do_get_info(
    const int pos MY_ATTRIBUTE((unused)), char *value MY_ATTRIBUTE((unused)),
    const size_t size MY_ATTRIBUTE((unused)),
    const char *default_value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
}

Rpl_info_handler::enum_field_get_status Rpl_info_dummy::do_get_info(
    const int pos MY_ATTRIBUTE((unused)), uchar *value MY_ATTRIBUTE((unused)),
    const size_t size MY_ATTRIBUTE((unused)),
    const uchar *default_value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
}

Rpl_info_handler::enum_field_get_status Rpl_info_dummy::do_get_info(
    const int pos MY_ATTRIBUTE((unused)), ulong *value MY_ATTRIBUTE((unused)),
    const ulong default_value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
}

Rpl_info_handler::enum_field_get_status Rpl_info_dummy::do_get_info(
    const int pos MY_ATTRIBUTE((unused)), int *value MY_ATTRIBUTE((unused)),
    const int default_value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
}

Rpl_info_handler::enum_field_get_status Rpl_info_dummy::do_get_info(
    const int pos MY_ATTRIBUTE((unused)), float *value MY_ATTRIBUTE((unused)),
    const float default_value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
}

Rpl_info_handler::enum_field_get_status Rpl_info_dummy::do_get_info(
    const int pos MY_ATTRIBUTE((unused)),
    Server_ids *value MY_ATTRIBUTE((unused)),
    const Server_ids *default_value MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(!abort);

  return Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
}

char *Rpl_info_dummy::do_get_description_info() {
  DBUG_ASSERT(!abort);

  return nullptr;
}

bool Rpl_info_dummy::do_is_transactional() {
  DBUG_ASSERT(!abort);

  return false;
}

bool Rpl_info_dummy::do_update_is_transactional() {
  DBUG_ASSERT(!abort);

  return false;
}
