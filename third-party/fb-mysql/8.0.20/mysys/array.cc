/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/array.cc
  Handling of arrays that can grow dynamically.
*/

#include <string.h>
#include <sys/types.h>

#include <algorithm>

#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/psi/psi_memory.h"
#include "mysql/service_mysql_alloc.h"

/*
  Initiate dynamic array

  SYNOPSIS
    my_init_dynamic_array()
      array		Pointer to an array
      element_size	Size of element
      init_buffer       Initial buffer pointer
      init_alloc	Number of initial elements
      alloc_increment	Increment for adding new elements

  DESCRIPTION
    init_dynamic_array() initiates array and allocate space for
    init_alloc eilements.
    Array is usable even if space allocation failed, hence, the
    function never returns true.
    Static buffers must begin immediately after the array structure.

  RETURN VALUE
    false	Ok
*/

bool my_init_dynamic_array(DYNAMIC_ARRAY *array, PSI_memory_key psi_key,
                           uint element_size, void *init_buffer,
                           uint init_alloc, uint alloc_increment) {
  DBUG_TRACE;
  if (!alloc_increment) {
    alloc_increment = std::max((8192 - MALLOC_OVERHEAD) / element_size, 16U);
    if (init_alloc > 8 && alloc_increment > init_alloc * 2)
      alloc_increment = init_alloc * 2;
  }

  if (!init_alloc) {
    init_alloc = alloc_increment;
    init_buffer = nullptr;
  }
  array->elements = 0;
  array->max_element = init_alloc;
  array->alloc_increment = alloc_increment;
  array->size_of_element = element_size;
  array->m_psi_key = psi_key;
  if ((array->buffer = static_cast<uchar *>(init_buffer))) return false;
  /*
    Since the dynamic array is usable even if allocation fails here malloc
    should not throw an error
  */
  if (!(array->buffer =
            (uchar *)my_malloc(psi_key, element_size * init_alloc, MYF(0))))
    array->max_element = 0;
  return false;
}

/*
  Insert element at the end of array. Allocate memory if needed.

  SYNOPSIS
    insert_dynamic()
      array
      element

  RETURN VALUE
    true	Insert failed
    false	Ok
*/

bool insert_dynamic(DYNAMIC_ARRAY *array, const void *element) {
  uchar *buffer;
  if (array->elements == array->max_element) { /* Call only when nessesary */
    if (!(buffer = static_cast<uchar *>(alloc_dynamic(array)))) return true;
  } else {
    buffer = array->buffer + (array->elements * array->size_of_element);
    array->elements++;
  }
  memcpy(buffer, element, (size_t)array->size_of_element);
  return false;
}

/*
  Alloc space for next element(s)

  SYNOPSIS
    alloc_dynamic()
      array

  DESCRIPTION
    alloc_dynamic() checks if there is empty space for at least
    one element if not tries to allocate space for alloc_increment
    elements at the end of array.

  RETURN VALUE
    pointer	Pointer to empty space for element
    0		Error
*/

void *alloc_dynamic(DYNAMIC_ARRAY *array) {
  if (array->elements == array->max_element) {
    char *new_ptr;
    if (array->buffer == (uchar *)(array + 1)) {
      /*
        In this senerio, the buffer is statically preallocated,
        so we have to create an all-new malloc since we overflowed
      */
      if (!(new_ptr = (char *)my_malloc(
                array->m_psi_key,
                (array->max_element + array->alloc_increment) *
                    array->size_of_element,
                MYF(MY_WME))))
        return nullptr;
      memcpy(new_ptr, array->buffer, array->elements * array->size_of_element);
    } else if (!(new_ptr = (char *)my_realloc(
                     array->m_psi_key, array->buffer,
                     (array->max_element + array->alloc_increment) *
                         array->size_of_element,
                     MYF(MY_WME | MY_ALLOW_ZERO_PTR))))
      return nullptr;
    array->buffer = (uchar *)new_ptr;
    array->max_element += array->alloc_increment;
  }
  return array->buffer + (array->elements++ * array->size_of_element);
}

/*
  Empty array by freeing all memory

  SYNOPSIS
    delete_dynamic()
      array	Array to be deleted
*/

void delete_dynamic(DYNAMIC_ARRAY *array) {
  /*
    Just mark as empty if we are using a static buffer
  */
  if (array->buffer == (uchar *)(array + 1))
    array->elements = 0;
  else if (array->buffer) {
    my_free(array->buffer);
    array->buffer = nullptr;
    array->elements = array->max_element = 0;
  }
}
