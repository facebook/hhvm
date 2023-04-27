/*
   Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef ABSTRACT_OPTIONS_PROVIDER_INCLUDED
#define ABSTRACT_OPTIONS_PROVIDER_INCLUDED

#include <map>
#include <string>

#include "client/base/bool_option.h"
#include "client/base/char_array_option.h"
#include "client/base/disabled_option.h"
#include "client/base/enum_option.h"
#include "client/base/i_option.h"
#include "client/base/i_option_changed_listener.h"
#include "client/base/i_options_provider.h"
#include "client/base/number_option.h"
#include "client/base/password_option.h"
#include "client/base/simple_option.h"
#include "client/base/string_option.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Base {
namespace Options {

/**
  Common abstract class for options providers.
  Provides common functionalities.
 */
class Abstract_options_provider : public I_options_provider {
 public:
  /**
    Creates and attach new simple option.
    @param name Name of option. It is used in command-line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Simple_option *create_new_option(std::string name, std::string description);
  /**
    Creates and attach new disabled option. This option is to mark existance
    of options inavailable due to distribution configuration.
    @param name Name of option. It is used in command-line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Disabled_option *create_new_disabled_option(std::string name,
                                              std::string description);
  /**
    Creates and attach new string option stored in char* type object.
    @param value Pointer to char* object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Char_array_option *create_new_option(char **value, std::string name,
                                       std::string description);
  /**
    Creates and attach new password option. It removes password from
    command-line on UNIX systems to prevent password to be seen when listing
    processes.
    @param value Pointer to Nullable<string> object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Password_option *create_new_password_option(Nullable<std::string> *value,
                                              std::string name,
                                              std::string description);
  /**
    Creates and attach new string option.
    @param value Pointer to Nullable<string> object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  String_option *create_new_option(Nullable<std::string> *value,
                                   std::string name, std::string description);
  /**
    Creates and attach new 32-bit signed number option.
    @param value Pointer to int32 object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option<int32> *create_new_option(int32 *value, std::string name,
                                          std::string description);
  /**
    Creates and attach new 32-bit unsigned number option.
    @param value Pointer to uint32 object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option<uint32> *create_new_option(uint32 *value, std::string name,
                                           std::string description);
  /**
    Creates and attach new 64-bit signed number option.
    @param value Pointer to int64 object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option<int64> *create_new_option(int64 *value, std::string name,
                                          std::string description);
  /**
    Creates and attach new 64-bit unsigned number option.
    @param value Pointer to uint64 object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option<uint64> *create_new_option(uint64 *value, std::string name,
                                           std::string description);
  /**
    Creates and attach new floating-point number option.
    @param value Pointer to double object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Number_option<double> *create_new_option(double *value, std::string name,
                                           std::string description);
  /**
    Creates and attach new boolean option with value received from argument.
    @param value Pointer to double object to receive option value.
    @param name Name of option. It is used in command line option name as
      --name.
    @param description Description of option to be printed in --help.
   */
  Bool_option *create_new_option(bool *value, std::string name,
                                 std::string description);

  template <typename T_type, typename T_typelib>
  Enum_option<T_type, T_typelib> *create_new_enum_option(
      T_type *value, const T_typelib *type, std::string name,
      std::string description) {
    return this->attach_new_option<Enum_option<T_type, T_typelib>>(
        new Enum_option<T_type, T_typelib>(value, type, name, description));
  }

  /**
    Creates all options that will be provided.
   */
  virtual void create_options() = 0;

  /**
    Creates list of options provided by this provider.
    Part of I_options_provider interface implementation.
    @returns list of my_getopt internal option data structures.
   */
  virtual std::vector<my_option> generate_options();
  /**
    Callback to be called when command-line options parsing have finished.
    Part of I_options_provider interface implementation.
   */
  virtual void options_parsed();

 protected:
  Abstract_options_provider();
  virtual ~Abstract_options_provider();

  /**
    Sets optional option changes listener to which all changes in all options
    contained in this provider should be reported. This is used when this
    provider is attached to another.
    Part of I_options_provider interface implementation.
   */
  virtual void set_option_changed_listener(I_option_changed_listener *listener);

 private:
  /**
    Makes sure this provider will be able to watch name and optid usage.
   */
  template <typename T_type>
  T_type *attach_new_option(T_type *option) {
    // Make this option reporting all name and optid changes to us.
    option->set_option_changed_listener(this);

    // Add to list of our own options.
    this->m_options_created.push_back(option);

    // Check for name and optid collision.
    this->notify_option_name_changed(option, "");
    this->notify_option_optid_changed(option, 0);

    return option;
  }

  /**
    Called after specified option has name changed.
    It is also called when new option is added, old_name is empty string in
    that case.
    Part of I_option_changed_listener interface implementation.
   */
  virtual void notify_option_name_changed(I_option *source,
                                          std::string old_name);
  /**
    Called after specified option has option ID changed.
    It is also called when new option is added, old_optid is 0 in that case.
    Part of I_option_changed_listener interface implementation.
   */
  virtual void notify_option_optid_changed(I_option *source, uint32 old_optid);

  bool m_are_options_created;
  std::map<std::string, I_option *> m_name_usage;
  std::map<uint32, I_option *> m_optid_usage;
  I_option_changed_listener *m_option_changed_listener;
  std::vector<I_option *> m_options_created;
};

}  // namespace Options
}  // namespace Base
}  // namespace Tools
}  // namespace Mysql

#endif
