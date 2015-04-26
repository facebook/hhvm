/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HHVM_XDEBUG_CHROME_H_
#define incl_HHVM_XDEBUG_CHROME_H_

#include <folly/Range.h>

#include <string>

namespace HPHP {
//////////////////////////////////////////////////////////////////////

struct xdebug_xml_node;

/* Converts a chrome debugger protocol /command/ to an dbgp /command/. */
std::string chrome_to_dbgp(folly::StringPiece);

/* Converts a dbgp /result/ to a chrome debugger protocol /result/. */
std::string dbgp_to_chrome(xdebug_xml_node*);

//////////////////////////////////////////////////////////////////////
}

#endif
