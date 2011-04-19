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

#ifndef __HPHP_IMMUTABLE_OBJ_H__
#define __HPHP_IMMUTABLE_OBJ_H__

#include <runtime/base/types.h>
#include <util/hash.h>
#include <util/atomic.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedVariant;
class SharedVariantStats;

class ImmutableObj {
public:
  ImmutableObj(ObjectData *obj);
  Object getObject();
  ~ImmutableObj();
  void getSizeStats(SharedVariantStats *stats);
  int32 getSpaceUsage();

private:
  struct Prop {
    StringData *name;
    SharedVariant *val;
  };
  Prop* m_props;
  int m_propCount;
  StringData *m_cls;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_IMMUTABLE_OBJ_H__ */
