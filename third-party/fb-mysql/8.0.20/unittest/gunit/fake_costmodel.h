#ifndef FAKE_COSTMODEL_INCLUDED
#define FAKE_COSTMODEL_INCLUDED

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

#include <stddef.h>

#include "sql/opt_costconstants.h"
#include "sql/opt_costmodel.h"

/**
  This is is a "fake" cost model that can be used in unit tests that
  do not link with the server libraries.
*/

class Fake_Cost_model_server : public Cost_model_server {
 public:
  Fake_Cost_model_server() {
    // Create default values for server cost constants
    m_server_cost_constants = new Server_cost_constants();
#if !defined(DBUG_OFF)
    m_initialized = true;
#endif
  }

  ~Fake_Cost_model_server() {
    delete m_server_cost_constants;
    m_server_cost_constants = nullptr;
  }
};

class Fake_Cost_model_table : public Cost_model_table {
 public:
  Fake_Cost_model_table() {
    // Create a fake cost model server object that will provide
    // cost constants for server operations
    m_cost_model_server = new Fake_Cost_model_server();

    // Allocate cost constants for operations on tables
    m_se_cost_constants = new SE_cost_constants();

#if !defined(DBUG_OFF)
    m_initialized = true;
#endif
  }

  ~Fake_Cost_model_table() {
    delete m_cost_model_server;
    m_cost_model_server = nullptr;
    delete m_se_cost_constants;
    m_se_cost_constants = nullptr;
  }
};

#endif /* FAKE_COSTMODEL_INCLUDED */
