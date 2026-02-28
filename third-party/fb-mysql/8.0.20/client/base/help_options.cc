/*
   Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "client/base/help_options.h"

#include <stdlib.h>
#include <functional>
#include <sstream>

#include "client/base/abstract_program.h"
#include "client/client_priv.h"
#include "my_default.h"
#include "print_version.h"
#include "welcome_copyright_notice.h" /* ORACLE_WELCOME_COPYRIGHT_NOTICE */

using namespace Mysql::Tools::Base::Options;
using Mysql::Tools::Base::Abstract_program;
using std::string;
using std::placeholders::_1;

extern const char *load_default_groups[];

Help_options::Help_options(Abstract_program *program) : m_program(program) {}

void Help_options::create_options() {
  this->create_new_option("help", "Display this help message and exit.")
      ->set_short_character('?')
      ->add_callback(new std::function<void(char *)>(
          std::bind(&Help_options::help_callback, this, _1)));

  this->create_new_option("version", "Output version information and exit.")
      ->set_short_character('V')
      ->add_callback(new std::function<void(char *)>(
          std::bind(&Help_options::version_callback, this, _1)));
}

void Help_options::help_callback(char *argument MY_ATTRIBUTE((unused))) {
  this->print_usage();
  exit(0);
}

void Help_options::version_callback(char *argument MY_ATTRIBUTE((unused))) {
  this->print_version_line();
  exit(0);
}

/** A helper function. Prints the program version line. */
void Help_options::print_version_line() { print_version(); }

void Mysql::Tools::Base::Options::Help_options::print_usage() {
  this->print_version_line();

  std::ostringstream s;
  s << m_program->get_first_release_year();
  string first_year_str(s.str());
  string copyright;

  if (first_year_str == COPYRIGHT_NOTICE_CURRENT_YEAR) {
    copyright = ORACLE_WELCOME_COPYRIGHT_NOTICE(COPYRIGHT_NOTICE_CURRENT_YEAR);
  } else {
#define FIRST_YEAR_CONSTANT "$first_year$"
    string first_year_constant_str = FIRST_YEAR_CONSTANT;

    copyright = ORACLE_WELCOME_COPYRIGHT_NOTICE(FIRST_YEAR_CONSTANT);
    copyright =
        copyright.replace(copyright.find(first_year_constant_str),
                          first_year_constant_str.length(), first_year_str);
  }

  printf("%s\n%s\n", copyright.c_str(),
         this->m_program->get_description().c_str());
  this->m_program->short_usage();
  print_defaults("my", load_default_groups);
  my_print_help(this->m_program->get_options_array());
  my_print_variables(this->m_program->get_options_array());
}
