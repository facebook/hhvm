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

#ifndef ABSTRACT_OPTION_INCLUDED
#define ABSTRACT_OPTION_INCLUDED

#include <functional>
#include <string>
#include <vector>

#include "client/base/i_option_changed_listener.h"
#include "my_dbug.h"
#include "my_getopt.h"
#include "mysql/service_mysql_alloc.h"

namespace Mysql {
namespace Tools {
namespace Base {
namespace Options {

class Abstract_options_provider;

/**
  Abstract base with common option functionalities.
 */
template <typename T_type>
class Abstract_option : public I_option {
 public:
  virtual ~Abstract_option();

  /**
    Adds new callback for this option for option_parsed() event to callback
    chain.
    I_Callable can be replaced with std::Function<void(char*)> once we get
    one.
   */
  void add_callback(std::function<void(char *)> *callback);

  /**
    Sets optid to given character to make possible usage of short option
    alternative.
   */
  T_type *set_short_character(char code);

 protected:
  /**
    Constructs new option.
    @param value Pointer to object to receive option value.
    @param var_type my_getopt internal option type.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
    @param default_value default value to be supplied to internal option
      data structure.
   */
  Abstract_option(void *value, ulong var_type, std::string name,
                  std::string description, longlong default_value);

  /**
    Returns my_getopt internal option data structure representing this option.
    To be used by Abstract_options_provider when preparing options array to
    return.
   */
  my_option get_my_option();

  /**
    Method to set listener on option changed events.
    For use from Abstract_options_provider class only.
   */
  void set_option_changed_listener(I_option_changed_listener *listener);

  my_option m_option_structure;

 private:
  void call_callbacks(char *argument);

  std::vector<std::function<void(char *)> *> m_callbacks;
  I_option_changed_listener *m_option_changed_listener;

  friend class Abstract_options_provider;
};

template <typename T_type>
Abstract_option<T_type>::~Abstract_option() {
  my_free(const_cast<char *>(m_option_structure.name));
  my_free(const_cast<char *>(m_option_structure.comment));

  for (std::vector<std::function<void(char *)> *>::iterator it =
           this->m_callbacks.begin();
       it != this->m_callbacks.end(); it++) {
    delete *it;
  }
}

template <typename T_type>
void Abstract_option<T_type>::add_callback(
    std::function<void(char *)> *callback) {
  this->m_callbacks.push_back(callback);
}

template <typename T_type>
T_type *Abstract_option<T_type>::set_short_character(char code) {
  // Change optid to new one
  uint32 old_optid = this->m_option_structure.id;
  this->m_option_structure.id = (int)code;

  // Inform that it has changed
  if (this->m_option_changed_listener != nullptr) {
    this->m_option_changed_listener->notify_option_optid_changed(this,
                                                                 old_optid);
  }

  return (T_type *)this;
}

template <typename T_type>
Abstract_option<T_type>::Abstract_option(void *value, ulong var_type,
                                         std::string name,
                                         std::string description,
                                         longlong default_value)
    : m_option_changed_listener(nullptr) {
  this->m_option_structure.block_size = 0;
  this->m_option_structure.max_value = 0;
  this->m_option_structure.min_value = 0;
  this->m_option_structure.typelib = nullptr;
  this->m_option_structure.u_max_value = nullptr;

  this->m_option_structure.app_type = this;
  this->m_option_structure.arg_type = REQUIRED_ARG;
  this->m_option_structure.comment =
      my_strdup(PSI_NOT_INSTRUMENTED, description.c_str(), MYF(MY_FAE));
  // This in future can be changed to atomic operation (compare_and_exchange)
  this->m_option_structure.id = Abstract_option::last_optid;
  Abstract_option::last_optid++;
  ;
  this->m_option_structure.def_value = default_value;

  this->m_option_structure.name =
      my_strdup(PSI_NOT_INSTRUMENTED, name.c_str(), MYF(MY_FAE));
  /*
   TODO mbabij 15-04-2014: this is based on previous usages of my_option.
   Everyone sets this the same as my_option::value, explain why.
   */
  this->m_option_structure.u_max_value = value;

  this->m_option_structure.value = value;
  this->m_option_structure.var_type = var_type;
  this->m_option_structure.arg_source = nullptr;
}

template <typename T_type>
my_option Abstract_option<T_type>::get_my_option() {
  return this->m_option_structure;
}

template <typename T_type>
void Abstract_option<T_type>::set_option_changed_listener(
    I_option_changed_listener *listener) {
  DBUG_ASSERT(this->m_option_changed_listener == nullptr);

  this->m_option_changed_listener = listener;
}

template <typename T_type>
void Abstract_option<T_type>::call_callbacks(char *argument) {
  std::vector<std::function<void(char *)> *>::iterator callback_it;
  for (callback_it = this->m_callbacks.begin();
       callback_it != this->m_callbacks.end(); callback_it++) {
    (**callback_it)(argument);
  }
}

}  // namespace Options
}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
