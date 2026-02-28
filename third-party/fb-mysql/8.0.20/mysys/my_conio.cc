/* Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file mysys/my_conio.cc
*/

#ifdef _WIN32

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysys_priv.h"

extern CHARSET_INFO my_charset_utf16le_bin;

/* Windows console handling */

/*
  TODO : Find a relationship between the following
         two macros and get rid of one.
*/

/* Maximum line length on Windows console */
#define MAX_CONSOLE_LINE_SIZE 65535

/*
  Maximum number of characters that can be entered
  on single line in the console (including \r\n).
*/
#define MAX_NUM_OF_CHARS_TO_READ 24530

/**
  Determine if a file is a windows console

  @param file Input stream

  @return
  @retval  0 if file is not Windows console
  @retval  1 if file is Windows console
*/
bool my_win_is_console(FILE *file) {
  DWORD mode;
  if (GetConsoleMode((HANDLE)_get_osfhandle(_fileno(file)), &mode)) return 1;
  return 0;
}

/**
  Read line from Windows console using Unicode API
  and translate input to session character set.
  Note, as Windows API breaks supplementary characters
  into two wchar_t pieces, we cannot read and convert individual
  wchar_t values separately. So let's use a buffer for
  Unicode console input, and then convert it to "cs" in a single shot.
  String is terminated with '\0' character.

  @param cs           Character string to convert to.
  @param [out] mbbuf  Write input data here.
  @param mbbufsize    Number of bytes available in mbbuf.
  @param [out] nread  Number of bytes read.

  @retval           Pointer to mbbuf, or NULL on I/0 error.
*/
char *my_win_console_readline(const CHARSET_INFO *cs, char *mbbuf,
                              size_t mbbufsize, size_t *nread) {
  uint dummy_errors;
  static wchar_t u16buf[MAX_CONSOLE_LINE_SIZE + 1];
  size_t mblen = 0;
  DWORD console_mode;
  DWORD nchars;

  HANDLE console = GetStdHandle(STD_INPUT_HANDLE);

  DBUG_ASSERT(mbbufsize > 0); /* Need space for at least trailing '\0' */
  GetConsoleMode(console, &console_mode);
  SetConsoleMode(
      console, ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT);

  if (!ReadConsoleW(console, u16buf, MAX_NUM_OF_CHARS_TO_READ, &nchars, NULL)) {
    SetConsoleMode(console, console_mode);
    return NULL;
  }

  *nread = nchars;

  /* Set length of string */
  if (nchars >= 2 && u16buf[nchars - 2] == L'\r')
    nchars -= 2;
  else if ((nchars == MAX_NUM_OF_CHARS_TO_READ) &&
           (u16buf[nchars - 1] == L'\r'))
    /* Special case 1 - \r\n straddles the boundary */
    nchars--;
  else if ((nchars == 1) && (u16buf[0] == L'\n'))
    /* Special case 2 - read a single '\n'*/
    nchars--;

  SetConsoleMode(console, console_mode);

  /* Convert Unicode to session character set */
  if (nchars != 0)
    mblen = my_convert(mbbuf, mbbufsize - 1, cs, (const char *)u16buf,
                       nchars * sizeof(wchar_t), &my_charset_utf16le_bin,
                       &dummy_errors);

  DBUG_ASSERT(mblen < mbbufsize); /* Safety */
  mbbuf[mblen] = 0;
  return mbbuf;
}

/**
  Translate client charset to Windows wchars for console I/O.
  Unlike copy_and_convert(), in case of a wrong multi-byte sequence
  we don't print '?' character, we fallback to ISO-8859-1 instead.
  This gives a better idea how binary data (e.g. BLOB) look like.

  @param cs           Character set of the input string
  @param from         Input string
  @param from_length  Length of the input string
  @param [out] to     Write Unicode data here
  @param to_chars     Number of characters available in "to"
*/
static size_t my_mbstou16s(const CHARSET_INFO *cs, const uchar *from,
                           size_t from_length, wchar_t *to, size_t to_chars) {
  const CHARSET_INFO *to_cs = &my_charset_utf16le_bin;
  const uchar *from_end = from + from_length;
  wchar_t *to_orig = to, *to_end = to + to_chars;
  my_charset_conv_mb_wc mb_wc = cs->cset->mb_wc;
  my_charset_conv_wc_mb wc_mb = to_cs->cset->wc_mb;
  while (from < from_end) {
    int cnvres;
    my_wc_t wc;
    if ((cnvres = (*mb_wc)(cs, &wc, from, from_end)) > 0) {
      if (!wc) break;
      from += cnvres;
    } else if (cnvres == MY_CS_ILSEQ) {
      wc = (my_wc_t)(uchar)*from; /* Fallback to ISO-8859-1 */
      from += 1;
    } else if (cnvres > MY_CS_TOOSMALL) {
      /*
        A correct multibyte sequence detected
        But it doesn't have Unicode mapping.
      */
      wc = '?';
      from += (-cnvres); /* Note: cnvres is negative here */
    } else               /* Incomplete character */
    {
      wc = (my_wc_t)(uchar)*from; /* Fallback to ISO-8859-1 */
      from += 1;
    }
  outp:
    if ((cnvres = (*wc_mb)(to_cs, wc, (uchar *)to, (uchar *)to_end)) > 0) {
      /* We can never convert only a part of wchar_t */
      DBUG_ASSERT((cnvres % sizeof(wchar_t)) == 0);
      /* cnvres returns number of bytes, convert to number of wchar_t's */
      to += cnvres / sizeof(wchar_t);
    } else if (cnvres == MY_CS_ILUNI && wc != '?') {
      wc = '?';
      goto outp;
    } else
      break; /* Not enough space */
  }
  return to - to_orig;
}

/**
  Write a string in the given character set to Windows console.
  As Window breaks supplementary characters into two parts,
  we cannot use a simple loop sending the result of
  cs->cset->mb_wc() to console.
  So we converts string from client charset to an array of wchar_t,
  then write the array to console in a single shot.

  @param cs       Character set of the string
  @param data     String to print
  @param datalen  Length of input string in bytes
*/
void my_win_console_write(const CHARSET_INFO *cs, const char *data,
                          size_t datalen) {
  static wchar_t u16buf[MAX_CONSOLE_LINE_SIZE + 1];
  size_t nchars =
      my_mbstou16s(cs, (const uchar *)data, datalen, u16buf, sizeof(u16buf));
  DWORD nwritten;
  WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), u16buf, (DWORD)nchars,
                &nwritten, NULL);
}

/**
  Write a single-byte character to console.
  Note: one should not send parts of the same multi-byte character
  in separate consequent my_win_console_putc() calls.
  For multi-byte characters use my_win_colsole_write() instead.

  @param cs  Character set of the input character
  @param c   Character (single byte)
*/
void my_win_console_putc(const CHARSET_INFO *cs, int c) {
  char ch = (char)c;
  my_win_console_write(cs, &ch, 1);
}

/**
  Write a 0-terminated string to Windows console.

  @param cs    Character set of the string to print
  @param data  String to print
*/
void my_win_console_fputs(const CHARSET_INFO *cs, const char *data) {
  my_win_console_write(cs, data, strlen(data));
}

/*
  Handle formatted output on the Windows console.
*/
void my_win_console_vfprintf(const CHARSET_INFO *cs, const char *fmt,
                             va_list args) {
  static char buff[MAX_CONSOLE_LINE_SIZE + 1];
  size_t len = vsnprintf(buff, sizeof(buff) - 1, fmt, args);
  my_win_console_write(cs, buff, len);
}

#include <shellapi.h>

/**
  Translate Unicode command line parameters to the given character set
  (Typically to utf8mb4).
  Translated parameters are allocated using my_once_alloc().

  @param      tocs    Character set to convert parameters to.
  @param[out] argc    Write number of parameters here
  @param[out] argv    Write pointer to allocated parameters here.
*/
int my_win_translate_command_line_args(const CHARSET_INFO *cs, int *argc,
                                       char ***argv) {
  int i, ac;
  char **av;
  wchar_t *command_line = GetCommandLineW();
  wchar_t **wargs = CommandLineToArgvW(command_line, &ac);
  size_t nbytes = (ac + 1) * sizeof(char *);

  /* Allocate new command line parameter */
  av = (char **)my_once_alloc(nbytes, MYF(MY_ZEROFILL));

  for (i = 0; i < ac; i++) {
    uint dummy_errors;
    size_t arg_len = wcslen(wargs[i]);
    size_t len, alloced_len = arg_len * cs->mbmaxlen + 1;
    av[i] = (char *)my_once_alloc(alloced_len, MYF(0));
    len = my_convert(av[i], alloced_len, cs, (const char *)wargs[i],
                     arg_len * sizeof(wchar_t), &my_charset_utf16le_bin,
                     &dummy_errors);
    DBUG_ASSERT(len < alloced_len);
    av[i][len] = '\0';
  }
  *argv = av;
  *argc = ac;
  /* Cleanup on exit */
  LocalFree((HLOCAL)wargs);
  return 0;
}

#endif /* _WIN32 */
