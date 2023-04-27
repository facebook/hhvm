#ifndef FAKE_COSTMODEL_CC_INCLUDED
#define FAKE_COSTMODEL_CC_INCLUDED

/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "unittest/gunit/fake_costmodel.h"

#include <sys/types.h>

#include "lex_string.h"
#include "m_string.h"
#include "my_dbug.h"

class THD;
struct TABLE;

Cost_model_server::~Cost_model_server() {}

const double Server_cost_constants::KEY_COMPARE_COST = 0.1;
const double Server_cost_constants::MEMORY_TEMPTABLE_CREATE_COST = 2.0;
const double Server_cost_constants::MEMORY_TEMPTABLE_ROW_COST = 0.2;
const double Server_cost_constants::DISK_TEMPTABLE_CREATE_COST = 40.0;
const double Server_cost_constants::DISK_TEMPTABLE_ROW_COST = 1.0;
const double Server_cost_constants::ROW_EVALUATE_COST = 0.2;
const double SE_cost_constants::MEMORY_BLOCK_READ_COST = 1.0;
const double SE_cost_constants::IO_BLOCK_READ_COST = 1.0;

/* purecov: begin inspected */
const SE_cost_constants *Cost_model_constants::get_se_cost_constants(
    const TABLE *) const {
  // This is only implemented in order to link the unit tests
  DBUG_ASSERT(false);
  return nullptr;
}
/* purecov: end */

/* purecov: begin inspected */
cost_constant_error SE_cost_constants::set(const LEX_CSTRING &, const double,
                                           bool) {
  // This is only implemented in order to link the unit tests
  DBUG_ASSERT(false);
  return COST_CONSTANT_OK;
}
/* purecov: end */

/* purecov: begin inspected */
Cost_model_se_info::~Cost_model_se_info() {
  // This is only implemented in order to link the unit tests
  DBUG_ASSERT(false);
}
/* purecov: end */

/* purecov: begin inspected */
Cost_model_constants::~Cost_model_constants() {
  // This is only implemented in order to link the unit tests
  DBUG_ASSERT(false);
}
/* purecov: end */

/* purecov: begin inspected */
uint Cost_model_constants::find_handler_slot_from_name(
    THD *, const LEX_CSTRING &) const {
  // This is only implemented in order to link the unit tests
  DBUG_ASSERT(false);
  return 0;
}
/* purecov: end */

#endif /* FAKE_COSTMODEL_CC_INCLUDED */
