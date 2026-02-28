#ifndef FAKE_KEY_INCLUDED
#define FAKE_KEY_INCLUDED

/*
   Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <gtest/gtest.h>

#include "my_dbug.h"
#include "sql/key.h"  // KEY

/**
  A fake class to make it easy to set up a KEY object.

  Note that in this version the KEY object is only initialized with necessary
  information to do operations on rec_per_key.
*/

class Fake_KEY : public KEY {
 public:
  /**
    Initialize the KEY object.

    Only member variables needed for the rec_per_key interface are
    currently initialized.

    @param key_parts_arg number of key parts this index should have
    @param unique        unique or non-unique key
  */
  Fake_KEY(unsigned int key_parts_arg, bool unique) {
    init(key_parts_arg, key_parts_arg, unique);
  }

  /**
    Initialize the KEY object.

    Only member variables needed for the rec_per_key interface are
    currently initialized.

    @param user_defined_parts_arg number of key parts defined for index
    @param actual_key_parts_arg number of additional primary key parts
                                included in index
    @param unique        unique or non-unique key
  */
  Fake_KEY(unsigned int user_defined_parts_arg,
           unsigned int actual_key_parts_arg, bool unique) {
    init(user_defined_parts_arg, actual_key_parts_arg, unique);
  }

  ~Fake_KEY() {
    // free the memory for the two rec_per_key arrays
    delete[] m_rec_per_key;
    delete[] m_rec_per_key_float;
  }

 private:
  /**
    Initialize the KEY object.

    Only member variables needed for the rec_per_key interface are
    currently initialized.

    @param user_defined_key_parts_arg number of key parts defined for index
    @param actual_key_parts_arg number of additional primary key parts
                                included in index
    @param unique        unique or non-unique key
  */
  void init(unsigned int user_defined_key_parts_arg,
            unsigned int actual_key_parts_arg, bool unique) {
    DBUG_ASSERT(user_defined_key_parts_arg > 0);
    DBUG_ASSERT(actual_key_parts_arg > 0);

    flags = 0;
    if (unique) flags |= HA_NOSAME;
    actual_flags = flags;

    user_defined_key_parts = user_defined_key_parts_arg;
    actual_key_parts = actual_key_parts_arg;

    // Allocate memory for the two rec_per_key arrays
    m_rec_per_key = new ulong[actual_key_parts];
    m_rec_per_key_float = new rec_per_key_t[actual_key_parts];
    set_rec_per_key_array(m_rec_per_key, m_rec_per_key_float);

    // Initialize the rec_per_key arrays with default/unknown value
    for (uint kp = 0; kp < actual_key_parts; kp++) {
      rec_per_key[kp] = 0;
      set_records_per_key(kp, REC_PER_KEY_UNKNOWN);
    }
  }

  // Storage for the two records per key arrays
  ulong *m_rec_per_key;
  rec_per_key_t *m_rec_per_key_float;
};

#endif /* FAKE_KEY_INCLUDED */
