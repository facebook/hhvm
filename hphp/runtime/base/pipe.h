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

#ifndef incl_HPHP_PIPE_H_
#define incl_HPHP_PIPE_H_

#include "hphp/runtime/base/plain-file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Wrapper around popen/pclose.
 */
class Pipe : public PlainFile {
public:
  DECLARE_OBJECT_ALLOCATION(Pipe);

  Pipe();
  virtual ~Pipe();

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassNameHook() const { return s_class_name; }

  // implementing File
  virtual bool open(CStrRef filename, CStrRef mode);
  virtual bool close();

private:
  bool closeImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PIPE_H_
