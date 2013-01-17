/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef HPHP_STREAM_WRAPPER_H
#define HPHP_STREAM_WRAPPER_H

#include <string>
#include <runtime/base/types.h>
#include <runtime/base/file/file.h>

#include <boost/noncopyable.hpp>

namespace HPHP { namespace Stream {
///////////////////////////////////////////////////////////////////////////////

class Wrapper : boost::noncopyable {
 public:
  void registerAs(const std::string &scheme);

  virtual File* open(CStrRef filename, CStrRef mode,
                     int options, CVarRef context) = 0;

  virtual ~Wrapper() {}
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // HPHP_STREAM_WRAPPER_REGISTRY_H
