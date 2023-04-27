#ifndef EXPECTED_WARNINGS_INCLUDED
#define EXPECTED_WARNINGS_INCLUDED

// Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

#include "client/mysqltest/warning.h"

#include <memory>
#include <vector>

/// Class representing any one of the following list.
///   * List of disabled warnings
///   * List of enabled warnings
class Expected_warnings {
 public:
  typedef std::vector<std::unique_ptr<Warning>>::iterator iterator;

  Expected_warnings() {}
  ~Expected_warnings() {}

  iterator begin() { return m_warnings.begin(); }
  iterator end() { return m_warnings.end(); }

  /// Return length of the list containing warnings.
  ///
  /// @retval Length value
  std::size_t count() { return m_warnings.size(); }

  /// Delete all warnings from the vector.
  void clear_list() { m_warnings.clear(); }

  /// Add a new warning to the existing list of warnings only if it
  /// doesn't exist.
  ///
  /// @param warning_code  Warning number
  /// @param warning_name  Warning name
  /// @param once_property Flag value representing the scope of a warning.
  void add_warning(std::uint32_t warning_code, const char *warning_name,
                   bool once_property);

  /// Remove a warning from the existing list of warnings if it exists.
  /// If "ONCE" argument is specified, don't remove the warning, set a
  /// flag to ignore disabling or enabling of it for the next statement
  /// only.
  ///
  /// @param warning_code  Warning number
  /// @param once_property Flag value representing the scope of a
  ///                      disabled warning
  void remove_warning(std::uint32_t warning_code, bool once_property);

  /// Update the list of disabled or enabled warnings.
  ///
  /// * Remove all the warnings which are disabled or enabled only for
  ///   one statement. These warnings are expired after the execution of
  ///   next statement.
  ///
  /// * Reset ignore_warning flag value to 0 if it set to 1.
  void update_list();

  /// Return a list of symbolic names of disabled or enabled warnings.
  ///
  /// @retval String containing symbolic names of disabled warnings
  std::string warnings_list();

 private:
  // List containing disabled or enabled warnings.
  std::vector<std::unique_ptr<Warning>> m_warnings;
};

#endif  // EXPECTED_WARNINGS_INCLUDED
