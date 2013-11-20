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

#ifndef HPHP_USER_FS_NODE_H
#define HPHP_USER_FS_NODE_H

#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {

class UserFSNode {
public:
  explicit UserFSNode(Class *cls, CVarRef context = uninit_null());

protected:
  Variant invoke(const Func *func, const String& name, CArrRef args,
                 bool &success);
  Variant invoke(const Func *func, const String& name, CArrRef args) {
    bool success;
    return invoke(func, name, args, success);
  }
  const Func* lookupMethod(const StringData* name);

protected:
  const Func* m_Call;
  Class* m_cls;

private:
  Object m_obj;
};

}

#endif // HPHP_USER_FS_NODE_H
