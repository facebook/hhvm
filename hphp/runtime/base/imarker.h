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
#ifndef incl_HPHP_RUNTIME_IMARKER_H_
#define incl_HPHP_RUNTIME_IMARKER_H_

namespace HPHP {
struct Array;
struct String;
struct Variant;
namespace req { template<typename T> struct ptr; }

// Interface for marking.
struct IMarker {
  virtual void operator()(const Array&) = 0;
  virtual void operator()(const String&) = 0;
  virtual void operator()(const Variant&) = 0;
  virtual void operator()(const void* start, size_t len) = 0;
protected:
  ~IMarker() {}
};

template <typename T, typename F>
void scan(const req::ptr<T>& ptr, F& mark);

}
#endif
