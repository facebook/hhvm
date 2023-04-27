/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _welcome_copyright_notice_h_
#define _welcome_copyright_notice_h_

/**
  @file include/welcome_copyright_notice.h
*/

#include <string.h>

#define COPYRIGHT_NOTICE_CURRENT_YEAR "2020"

/*
  This define specifies copyright notice which is displayed by every MySQL
  program on start, or on help screen.
*/
#define ORACLE_WELCOME_COPYRIGHT_NOTICE(first_year)                            \
  (strcmp(first_year, COPYRIGHT_NOTICE_CURRENT_YEAR)                           \
       ? "Copyright (c) " first_year ", " COPYRIGHT_NOTICE_CURRENT_YEAR        \
         ", "                                                                  \
         "Oracle and/or its affiliates. All rights reserved.\n\nOracle is a "  \
         "registered trademark of Oracle Corporation and/or its\naffiliates. " \
         "Other names may be trademarks of their respective\nowners.\n"        \
       : "Copyright (c) " first_year                                           \
         ", Oracle and/or its affiliates. "                                    \
         "All rights reserved.\n\nOracle is a registered trademark of "        \
         "Oracle Corporation and/or its\naffiliates. Other names may be "      \
         "trademarks of their respective\nowners.\n")

#define ORACLE_GPL_LICENSE_TEXT                                               \
  "   This program is free software; you can redistribute it and/or modify\n" \
  "   it under the terms of the GNU General Public License, version 2.0,\n"   \
  "   as published by the Free Software Foundation.\n"                        \
  "\n"                                                                        \
  "   This program is also distributed with certain software (including\n"    \
  "   but not limited to OpenSSL) that is licensed under separate terms,\n"   \
  "   as designated in a particular file or component or in included "        \
  "license\n"                                                                 \
  "   documentation.  The authors of MySQL hereby grant you an additional\n"  \
  "   permission to link the program and your derivative works with the\n"    \
  "   separately licensed software that they have included with MySQL.\n"     \
  "\n"                                                                        \
  "   This program is distributed in the hope that it will be useful,\n"      \
  "   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"       \
  "   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"        \
  "   GNU General Public License, version 2.0, for more details.\n"           \
  "\n"                                                                        \
  "   You should have received a copy of the GNU General Public License\n"    \
  "   along with this program; if not, write to the Free Software\n"          \
  "   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  " \
  "USA */\n"

#define ORACLE_COPYRIGHT_NOTICE(first_year)                                \
  (strcmp(first_year, COPYRIGHT_NOTICE_CURRENT_YEAR)                       \
       ? "/* Copyright (c) " first_year ", " COPYRIGHT_NOTICE_CURRENT_YEAR \
         ", Oracle and/or its affiliates. All rights reserved. */\n"       \
         "\n"                                                              \
       : "/* Copyright (c) " first_year                                    \
         ", Oracle and/or its affiliates. All rights reserved. */\n")

#define ORACLE_GPL_COPYRIGHT_NOTICE(first_year)                            \
  (strcmp(first_year, COPYRIGHT_NOTICE_CURRENT_YEAR)                       \
       ? "/* Copyright (c) " first_year ", " COPYRIGHT_NOTICE_CURRENT_YEAR \
         ", Oracle and/or its affiliates. All rights reserved.\n"          \
         "\n" ORACLE_GPL_LICENSE_TEXT                                      \
       : "/* Copyright (c) " first_year                                    \
         ", Oracle and/or its affiliates. All rights reserved.\n"          \
         "\n" ORACLE_GPL_LICENSE_TEXT)

#define ORACLE_GPL_FOSS_LICENSE_TEXT                                          \
  "   This program is free software; you can redistribute it and/or modify\n" \
  "   it under the terms of the GNU General Public License, version 2.0,\n"   \
  "   as published by the Free Software Foundation.\n"                        \
  "\n"                                                                        \
  "   This program is also distributed with certain software (including\n"    \
  "   but not limited to OpenSSL) that is licensed under separate terms,\n"   \
  "   as designated in a particular file or component or in included "        \
  "license\n"                                                                 \
  "   documentation.  The authors of MySQL hereby grant you an additional\n"  \
  "   permission to link the program and your derivative works with the\n"    \
  "   separately licensed software that they have included with MySQL.\n"     \
  "\n"                                                                        \
  "   Without limiting anything contained in the foregoing, this file,\n"     \
  "   which is part of C Driver for MySQL (Connector/C), is also subject to " \
  "the\n"                                                                     \
  "   Universal FOSS Exception, version 1.0, a copy of which can be found "   \
  "at\n"                                                                      \
  "   http://oss.oracle.com/licenses/universal-foss-exception.\n"             \
  "\n"                                                                        \
  "   This program is distributed in the hope that it will be useful,\n"      \
  "   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"       \
  "   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"        \
  "   GNU General Public License, version 2.0, for more details.\n"           \
  "\n"                                                                        \
  "   You should have received a copy of the GNU General Public License\n"    \
  "   along with this program; if not, write to the Free Software\n"          \
  "   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  " \
  "USA */\n"

#define ORACLE_GPL_FOSS_COPYRIGHT_NOTICE(first_year)                       \
  (strcmp(first_year, COPYRIGHT_NOTICE_CURRENT_YEAR)                       \
       ? "/* Copyright (c) " first_year ", " COPYRIGHT_NOTICE_CURRENT_YEAR \
         ", Oracle and/or its affiliates. All rights reserved.\n"          \
         "\n" ORACLE_GPL_FOSS_LICENSE_TEXT                                 \
       : "/* Copyright (c) " first_year                                    \
         ", Oracle and/or its affiliates. All rights reserved.\n"          \
         "\n" ORACLE_GPL_FOSS_LICENSE_TEXT)

#endif /* _welcome_copyright_notice_h_ */
