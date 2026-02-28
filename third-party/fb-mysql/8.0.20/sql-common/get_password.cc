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

#include "my_config.h"

/*
** Ask for a password from tty
** This is an own file to avoid conflicts with curses
*/
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql.h"
#include "mysql/service_mysql_alloc.h"

#ifdef HAVE_GETPASS
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif /* HAVE_PWD_H */
#else  /* ! HAVE_GETPASS */
#if !defined(_WIN32)
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_TERMIOS_H /* For tty-password */
#include <termios.h>

#define TERMIO struct termios
#else
#ifdef HAVE_TERMIO_H /* For tty-password */
#include <termio.h>

#define TERMIO struct termio
#else
#include <sgtty.h>

#define TERMIO struct sgttyb
#endif
#endif
#else
#include <conio.h>
#endif /* _WIN32 */
#endif /* HAVE_GETPASS */

#ifdef HAVE_GETPASSPHRASE /* For Solaris */
#define getpass(A) getpassphrase(A)
#endif

#if defined(_WIN32)
/* were just going to fake it here and get input from the keyboard */
char *get_tty_password(const char *opt_message) {
  char to[80];
  char *pos = to, *end = to + sizeof(to) - 1;
  int i = 0;
  DBUG_TRACE;
  _cputs(opt_message ? opt_message : "Enter password: ");
  for (;;) {
    char tmp;
    tmp = _getch();
    if (tmp == '\b' || (int)tmp == 127) {
      if (pos != to) {
        _cputs("\b \b");
        pos--;
        continue;
      }
    }
    if (tmp == '\n' || tmp == '\r' || tmp == 3) break;
    if (iscntrl(tmp) || pos == end) continue;
    _cputs("*");
    *(pos++) = tmp;
  }
  while (pos != to && isspace(pos[-1]) == ' ')
    pos--; /* Allow dummy space at end */
  *pos = 0;
  _cputs("\n");
  return my_strdup(PSI_NOT_INSTRUMENTED, to, MYF(MY_FAE));
}

#else

#ifndef HAVE_GETPASS
/*
  Can't use fgets, because readline will get confused
  length is max number of chars in to, not counting \0
  to will not include the eol characters.
*/

static void get_password(char *to, uint length, int fd, bool echo) {
  char *pos = to, *end = to + length;

  for (;;) {
    char tmp;
    if (my_read(fd, &tmp, 1, MYF(0)) != 1) break;
    if (tmp == '\b' || (int)tmp == 127) {
      if (pos != to) {
        if (echo) {
          fputs("\b \b", stdout);
          fflush(stdout);
        }
        pos--;
        continue;
      }
    }
    if (tmp == '\n' || tmp == '\r' || tmp == 3) break;
    if (iscntrl(tmp) || pos == end) continue;
    if (echo) {
      fputc('*', stdout);
      fflush(stdout);
    }
    *(pos++) = tmp;
  }
  while (pos != to && isspace(pos[-1]) == ' ')
    pos--; /* Allow dummy space at end */
  *pos = 0;
  return;
}
#endif /* ! HAVE_GETPASS */

char *get_tty_password(const char *opt_message) {
#ifdef HAVE_GETPASS
  char *passbuff;
#else  /* ! HAVE_GETPASS */
  TERMIO org, tmp;
#endif /* HAVE_GETPASS */
  char buff[80];

  DBUG_TRACE;

#ifdef HAVE_GETPASS
  passbuff = getpass(opt_message ? opt_message : "Enter password: ");

  /* copy the password to buff and clear original (static) buffer */
  strncpy(buff, passbuff, sizeof(buff) - 1);
  buff[sizeof(buff) - 1] = 0;
#ifdef _PASSWORD_LEN
  memset(passbuff, 0, _PASSWORD_LEN);
#endif
#else
  if (isatty(fileno(stdout))) {
    fputs(opt_message ? opt_message : "Enter password: ", stdout);
    fflush(stdout);
  }
#if defined(HAVE_TERMIOS_H)
  tcgetattr(fileno(stdin), &org);
  tmp = org;
  tmp.c_lflag &= ~(ECHO | ISIG | ICANON);
  tmp.c_cc[VMIN] = 1;
  tmp.c_cc[VTIME] = 0;
  tcsetattr(fileno(stdin), TCSADRAIN, &tmp);
  get_password(buff, sizeof(buff) - 1, fileno(stdin), isatty(fileno(stdout)));
  tcsetattr(fileno(stdin), TCSADRAIN, &org);
#elif defined(HAVE_TERMIO_H)
  ioctl(fileno(stdin), (int)TCGETA, &org);
  tmp = org;
  tmp.c_lflag &= ~(ECHO | ISIG | ICANON);
  tmp.c_cc[VMIN] = 1;
  tmp.c_cc[VTIME] = 0;
  ioctl(fileno(stdin), (int)TCSETA, &tmp);
  get_password(buff, sizeof(buff) - 1, fileno(stdin), isatty(fileno(stdout)));
  ioctl(fileno(stdin), (int)TCSETA, &org);
#else
  gtty(fileno(stdin), &org);
  tmp = org;
  tmp.sg_flags &= ~ECHO;
  tmp.sg_flags |= RAW;
  stty(fileno(stdin), &tmp);
  get_password(buff, sizeof(buff) - 1, fileno(stdin), isatty(fileno(stdout)));
  stty(fileno(stdin), &org);
#endif
  if (isatty(fileno(stdout))) fputc('\n', stdout);
#endif /* HAVE_GETPASS */

  /*
    If the password is 79 bytes or longer, terminate the password by
    setting the last but one character to the null character.
  */
  buff[sizeof(buff) - 1] = '\0';
  return my_strdup(PSI_NOT_INSTRUMENTED, buff, MYF(MY_FAE));
}
#endif /* _WIN32 */
