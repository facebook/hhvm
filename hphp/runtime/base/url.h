/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_URL_H_
#define incl_HPHP_URL_H_

#include <string>

namespace HPHP {

namespace URL {
///////////////////////////////////////////////////////////////////////////////
/**
 * We define a "server object" as the part of URL without domain name:
 *
 *   http://facebook.com/foo?x=1       server object is "/foo?x=1"
 *   http://facebook.com/foo/bar?x=1   server object is "/foo/bar?x=1"
 */
const char *getServerObject(const char* url);

/**
 * We define a "command" as the part of URL without parameters:
 *
 *   /foo?x=1      command is "foo"
 *   foo?x=1       command is "foo"
 *   foo/bar?x=1   command is "foo/bar"
 *   /foo/bar?x=1  command is "foo/bar"
 */
std::string getCommand(const char* serverObject);

}  // namespace URL

///////////////////////////////////////////////////////////////////////////////
}  // namespace HPHP

#endif // incl_HPHP_URL_H_

