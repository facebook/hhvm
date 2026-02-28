#ifndef WARNING_INCLUDED
#define WARNING_INCLUDED

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

/// @file
///
/// This file declares the Warning class.

#include <cstdint>
#include <string>

/// Class representing a warning.
///
/// Contains following information
///   * A flag representing whether to ignore a warning or not
///   * A flag representing the scope of a warning
///   * Warning name
///   * Warning number
class Warning {
 public:
  Warning(std::uint32_t warning_code, const char *warning_name, bool once) {
    this->m_ignore_warning = false;
    this->m_once_property = once;
    this->m_warning_code = warning_code;
    this->m_warning_name.assign(warning_name);
  }

  /// Check if a warning is disabled/enabled for next statement only.
  ///
  /// @retval True if the warning is disabled or enabled for next
  ///         statement only, false otherwise.
  bool expired() { return m_once_property; }

  /// Return ignore_warning flag value
  ///
  /// @retval True if ignore_warning flag is set, false otherwise.
  bool ignore_warning() { return m_ignore_warning; }

  /// Return a symbolic name representing a warning
  ///
  /// @retval Symbolic name for a warning.
  const char *warning_name() { return m_warning_name.c_str(); }

  /// Return a warning code
  ///
  /// @retval Warning code
  std::uint32_t warning_code() { return m_warning_code; }

  /// Set ignore_warning flag
  ///
  /// @param value Boolean value for ignore_warning flag
  void set_ignore_warning(bool value) { m_ignore_warning = value; }

 private:
  bool m_ignore_warning;
  bool m_once_property;
  std::string m_warning_name;
  std::uint32_t m_warning_code;
};

#endif  // WARNING_INCLUDED
