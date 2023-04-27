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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "client/base/abstract_program.h"

#include <stdlib.h>
#include <algorithm>

#include "client/client_priv.h"
#include "my_default.h"

using namespace Mysql::Tools::Base;
using std::string;
using std::vector;

extern const char *load_default_groups[];

bool Abstract_program::callback_option_parsed(int, const struct my_option *opt,
                                              char *argument) {
  // Check if option uses My::Tools::Base::Options, and it should.
  Options::I_option *app_type = (Options::I_option *)opt->app_type;
  Options::I_option *option = dynamic_cast<Options::I_option *>(app_type);
  if (option != nullptr) {
    option->call_callbacks(argument);
  }
  return false;
}

const string Abstract_program::get_name() { return this->m_name; }

my_option *Abstract_program::get_options_array() { return &this->m_options[0]; }

Abstract_program::Abstract_program()
    : m_debug_options(this), m_help_options(this) {
  this->add_providers(&this->m_help_options, &this->m_debug_options, NULL);
}

void Abstract_program::run(int argc, char **argv) {
  this->init_name(argv[0]);

  MY_INIT(this->m_name.c_str());

  this->aggregate_options();

  my_getopt_use_args_separator = true;
  if (load_defaults("my", load_default_groups, &argc, &argv, &m_argv_alloc))
    this->error(Message_data(1, "Error during loading default options",
                             Message_type_error));
  my_getopt_use_args_separator = false;

  int ho_error = handle_options(&argc, &argv, this->get_options_array(),
                                Abstract_program::callback_option_parsed);
  if (ho_error != 0) {
    this->error(Message_data(ho_error, "Error during handling options",
                             Message_type_error));
  }

  // Let providers handle their parsed options.
  this->options_parsed();

  vector<string> positional_options;
  for (; argc > 0; argc--, argv++) {
    positional_options.push_back(*argv);
  }

  // Execute main body of program.
  int result = this->execute(positional_options);

  exit(result);
}

Abstract_program::~Abstract_program() {}

void Abstract_program::init_name(char *name_from_cmd_line) {
#ifdef _WIN32
  char *name;

  char name_buf[FN_REFLEN];
  if (GetModuleFileName(NULL, name_buf, FN_REFLEN) != 0) {
    name = name_buf;
  } else {
    name = name_from_cmd_line;
  }

  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];
  _splitpath_s(name, drive, dir, fname, ext);

  this->m_name = fname;
#else
  string name = name_from_cmd_line;
  this->m_name = name.substr(name.find_last_of('/') + 1);
#endif
}

void Abstract_program::aggregate_options() {
  // Concatenate all available command line options.
  this->m_options.clear();

  this->m_options = this->generate_options();

  // Sort by lexical order of long names.
  std::sort(this->m_options.begin(), this->m_options.end(),
            &Abstract_program::options_by_name_comparer);

  // Adding sentinel, handle_options assume input as array with sentinel.
  my_option sentinel = {nullptr, 0,          nullptr, nullptr, nullptr,
                        nullptr, GET_NO_ARG, NO_ARG,  0,       0,
                        0,       nullptr,    0,       nullptr};
  this->m_options.push_back(sentinel);
}

bool Abstract_program::options_by_name_comparer(const my_option &a,
                                                const my_option &b) {
  if (strcmp(a.name, "help") == 0) return true;
  if (strcmp(b.name, "help") == 0) return false;
  return strcmp(a.name, b.name) < 0;
}
