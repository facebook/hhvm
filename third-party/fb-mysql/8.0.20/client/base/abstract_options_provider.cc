/*
   Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "client/base/abstract_options_provider.h"

#include <stdlib.h>
#include <iostream>
#include <vector>

#include "client/base/i_options_provider.h"
#include "my_dbug.h"
#include "my_getopt.h"

using std::map;
using std::string;
using std::vector;
using namespace Mysql::Tools::Base::Options;
using Mysql::Nullable;

Simple_option *Abstract_options_provider::create_new_option(
    std::string name, std::string description) {
  return this->attach_new_option<Simple_option>(
      new Simple_option(name, description));
}

Disabled_option *Abstract_options_provider::create_new_disabled_option(
    std::string name, std::string description) {
  return this->attach_new_option<Disabled_option>(
      new Disabled_option(name, description));
}

Char_array_option *Abstract_options_provider::create_new_option(
    char **value, std::string name, std::string description) {
  return this->attach_new_option<Char_array_option>(
      new Char_array_option(value, false, name, description));
}

Password_option *Abstract_options_provider::create_new_password_option(
    Nullable<string> *value, std::string name, std::string description) {
  return this->attach_new_option<Password_option>(
      new Password_option(value, name, description));
}

String_option *Abstract_options_provider::create_new_option(
    Nullable<std::string> *value, std::string name, std::string description) {
  return this->attach_new_option<String_option>(
      new String_option(value, name, description));
}

Number_option<int32> *Abstract_options_provider::create_new_option(
    int32 *value, std::string name, std::string description) {
  return this->attach_new_option<Number_option<int32>>(
      new Number_option<int32>(value, name, description));
}

Number_option<uint32> *Abstract_options_provider::create_new_option(
    uint32 *value, std::string name, std::string description) {
  return this->attach_new_option<Number_option<uint32>>(
      new Number_option<uint32>(value, name, description));
}

Number_option<int64> *Abstract_options_provider::create_new_option(
    int64 *value, std::string name, std::string description) {
  return this->attach_new_option<Number_option<int64>>(
      new Number_option<int64>(value, name, description));
}

Number_option<uint64> *Abstract_options_provider::create_new_option(
    uint64 *value, std::string name, std::string description) {
  return this->attach_new_option<Number_option<uint64>>(
      new Number_option<uint64>(value, name, description));
}

Number_option<double> *Abstract_options_provider::create_new_option(
    double *value, std::string name, std::string description) {
  return this->attach_new_option<Number_option<double>>(
      new Number_option<double>(value, name, description));
}

Bool_option *Abstract_options_provider::create_new_option(
    bool *value, std::string name, std::string description) {
  return this->attach_new_option<Bool_option>(
      new Bool_option(value, name, description));
}

vector<my_option> Abstract_options_provider::generate_options() {
  if (this->m_are_options_created == false) {
    this->m_are_options_created = true;
    this->create_options();
  }

  vector<my_option> res;
  for (vector<I_option *>::iterator it = this->m_options_created.begin();
       it != this->m_options_created.end(); it++) {
    res.push_back((*it)->get_my_option());
  }

  return res;
}

void Abstract_options_provider::options_parsed() {}

Abstract_options_provider::Abstract_options_provider()
    : m_are_options_created(false), m_option_changed_listener(nullptr) {}

Abstract_options_provider::~Abstract_options_provider() {
  for (vector<I_option *>::iterator it = this->m_options_created.begin();
       it != this->m_options_created.end(); it++) {
    delete *it;
  }
}

void Abstract_options_provider::set_option_changed_listener(
    I_option_changed_listener *listener) {
  DBUG_ASSERT(this->m_option_changed_listener == nullptr);
  this->m_option_changed_listener = listener;
}

void Abstract_options_provider::notify_option_name_changed(I_option *source,
                                                           string old_name) {
  // Check if it is modification or new assignment
  if (old_name != "") {
    this->m_name_usage.erase(this->m_name_usage.find(old_name));
  }

  string new_name = source->get_my_option().name;

  // Try to find existing option with that name.
  map<string, I_option *>::iterator name_item =
      this->m_name_usage.find(new_name);

  // Report error if already used.
  if (name_item != this->m_name_usage.end()) {
    std::cerr << "Cannot register new option \"" << new_name
              << "\" as it collides with existing one with following name \""
              << name_item->second->get_my_option().name
              << "\" and description: "
              << name_item->second->get_my_option().comment << std::endl;
    exit(1);
  }
  // Add name usage.
  this->m_name_usage.insert(std::make_pair(new_name, source));

  // If we have listener we should inform it too.
  if (this->m_option_changed_listener != nullptr) {
    this->m_option_changed_listener->notify_option_name_changed(source,
                                                                old_name);
  }
}

void Abstract_options_provider::notify_option_optid_changed(I_option *source,
                                                            uint32 old_optid) {
  // Check if it is modification or new assignment
  if (old_optid != 0) {
    this->m_optid_usage.erase(this->m_optid_usage.find(old_optid));
  }

  uint32 new_optid = source->get_my_option().id;

  // Try to find existing option with that optid.
  map<uint32, I_option *>::iterator optid_item =
      this->m_optid_usage.find(new_optid);

  // Report error if already used.
  if (optid_item != this->m_optid_usage.end()) {
    string name = source->get_my_option().name;

    std::cerr << "Cannot register new option \"" << name
              << "\" as it collides with existing one with following name \""
              << optid_item->second->get_my_option().name
              << "\" and description: "
              << optid_item->second->get_my_option().comment << std::endl;
    exit(1);
  }
  // Add optid usage.
  this->m_optid_usage.insert(std::make_pair(new_optid, source));

  // If we have listener we should inform it too.
  if (this->m_option_changed_listener != nullptr) {
    this->m_option_changed_listener->notify_option_optid_changed(source,
                                                                 old_optid);
  }
}
