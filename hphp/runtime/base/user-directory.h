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

#ifndef HPHP_USER_DIRECTORY_H
#define HPHP_USER_DIRECTORY_H

#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/user-fs-node.h"

namespace HPHP {

class UserDirectory : public Directory, public UserFSNode {
public:
  CLASSNAME_IS("UserDirectory")
  DECLARE_RESOURCE_ALLOCATION(UserDirectory)

  explicit UserDirectory(Class* cls);
  ~UserDirectory() {}

  bool open(const String& path);
  void close() override;
  Variant read() override;
  void rewind() override;

private:
  const Func* m_DirOpen;
  const Func* m_DirClose;
  const Func* m_DirRead;
  const Func* m_DirRewind;
};

}

#endif // HPHP_USER_DIRECTORY_H
