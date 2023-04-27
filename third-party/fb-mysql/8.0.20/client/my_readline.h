#ifndef CLIENT_MY_READLINE_INCLUDED
#define CLIENT_MY_READLINE_INCLUDED

/*
   Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

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

/* readline for batch mode */

#include "my_inttypes.h"
#include "my_io.h"

struct LINE_BUFFER {
  File file;
  char *buffer; /* The buffer itself, grown as needed. */
  char *end;    /* Pointer at buffer end */
  char *start_of_line, *end_of_line;
  uint bufread; /* Number of bytes to get with each read(). */
  uint eof;
  ulong max_size;
  ulong read_length; /* Length of last read string */
  int error;
  bool truncated;
};

extern LINE_BUFFER *batch_readline_init(ulong max_size, FILE *file);
extern LINE_BUFFER *batch_readline_command(LINE_BUFFER *buffer, char *str);
extern char *batch_readline(LINE_BUFFER *buffer, bool binary_mode);
extern void batch_readline_end(LINE_BUFFER *buffer);

static const unsigned long int batch_io_size = 16 * 1024 * 1024;

#endif /* CLIENT_MY_READLINE_INCLUDED */
