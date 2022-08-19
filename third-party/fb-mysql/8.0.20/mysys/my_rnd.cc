/*
   Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_rnd.cc
*/

#include "my_rnd.h"
#include <mysql_com.h>
#include <openssl/err.h>
#include <openssl/rand.h>

/*
  A wrapper to use OpenSSL PRNGs.
*/

/**
  Generate random number.

  @param [in,out] rand_st Structure used for number generation.

  @retval                Generated pseudo random number.
*/

double my_rnd(struct rand_struct *rand_st) {
  rand_st->seed1 = (rand_st->seed1 * 3 + rand_st->seed2) % rand_st->max_value;
  rand_st->seed2 = (rand_st->seed1 + rand_st->seed2 + 33) % rand_st->max_value;
  return (((double)rand_st->seed1) / rand_st->max_value_dbl);
}

/**
Fill a buffer with random bytes using the SSL library routines

@param [out] buffer          Buffer to receive the random data
@param [in] buffer_size      sizeof the the buffer

@retval      1  error occurred.
@retval      0  OK
*/
int my_rand_buffer(unsigned char *buffer, size_t buffer_size) {
  int rc;
  rc = RAND_bytes(buffer, (int)buffer_size);

  if (!rc) {
    ERR_clear_error();
    return 1;
  }
  return 0;
}

/**
  Generate a random number using the OpenSSL supplied
  random number generator if available.

  @param [out] failed set to TRUE if the method failed. FALSE if OK.

  @retval                Generated random number or 0 if failed is set.
*/

double my_rnd_ssl(bool *failed) {
  unsigned int res;

  if (my_rand_buffer((unsigned char *)&res, sizeof(res))) {
    *failed = true;
    return 0;
  } else
    *failed = false;
  return (double)res / (double)UINT_MAX;
}
