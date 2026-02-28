/*
   Copyright (c) 2012, 2014, Oracle and/or its affiliates. All rights reserved.

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
#ifndef INFIX_OSTREAM_IT_INCLUDED
#define INFIX_OSTREAM_IT_INCLUDED
#include <iterator>
#include <ostream>
#include <string>

template <class T>
class infix_ostream_iterator
    : public std::iterator<std::output_iterator_tag, T> {
 public:
  infix_ostream_iterator(std::ostream &s) : m_os(&s) {}

  infix_ostream_iterator(std::ostream &s, const char *d)
      : m_os(&s), m_delimiter(d) {}

  infix_ostream_iterator<T> &operator=(T const &item) {
    *m_os << m_curr_delimiter << item;
    m_curr_delimiter = m_delimiter;
    return *this;
  }

  infix_ostream_iterator<T> &operator*() { return *this; }

  infix_ostream_iterator<T> &operator++() { return *this; }

  infix_ostream_iterator<T> &operator++(int) { return *this; }

 private:
  std::ostream *m_os;
  std::string m_curr_delimiter;
  std::string m_delimiter;
};

#endif
