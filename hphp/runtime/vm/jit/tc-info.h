/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_SERVER_TC_INFO_H_
#define incl_HPHP_SERVER_TC_INFO_H_

#include <string>
#include <vector>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct UsageInfo {
  std::string name;
  size_t used;
  size_t capacity;
  bool global;
};

/*
 * Get UsageInfo data for all the TC code sections, including global data, and
 * also for RDS.
 */
std::vector<UsageInfo> getUsageInfo();

/*
 * Like getUsageInfo(), but formatted as a pleasant string.
 */
std::string getTCSpace();

/*
 * Return a string containing the names and start addresses of all TC code
 * sections.
 */
std::string getTCAddrs();

///////////////////////////////////////////////////////////////////////////////

}}

#endif
