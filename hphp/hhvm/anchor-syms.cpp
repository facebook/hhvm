/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "hphp/runtime/ext/extension.h"

namespace HPHP {

/**
 * Prevent over-eager linkers from stripping seemingly unused
 * symbols from the resulting binary by linking them from here.
 */
extern Extension s_zip_extension;
extern Extension s_fileinfo_extension;
extern Extension s_intl_extension;
#ifdef HAVE_UODBC
extern Extension s_odbc_extension;
#endif
const Extension *g_anchor_extensions[] = {
  &s_zip_extension,
  &s_fileinfo_extension,
  &s_intl_extension,
#ifdef HAVE_UODBC
  &s_odbc_extension,
#endif
};

} // HPHP
