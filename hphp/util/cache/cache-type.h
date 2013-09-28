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

// See if this looks like the right sort of file.  Provided for
// migration purposes and is not supposed to stay around once the
// old cache code is removed.

#ifndef incl_HPHP_CACHE_TYPE_H_
#define incl_HPHP_CACHE_TYPE_H_

#include <string>

#include <boost/utility.hpp>

namespace HPHP {

class CacheType : private boost::noncopyable {
 public:
  CacheType();
  ~CacheType();

  bool isNewCache(const std::string& filename);
};

}  // namespace HPHP

#endif  // incl_HPHP_CACHE_TYPE_H_
