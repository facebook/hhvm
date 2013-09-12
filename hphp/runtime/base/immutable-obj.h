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

#ifndef incl_HPHP_IMMUTABLE_OBJ_H_
#define incl_HPHP_IMMUTABLE_OBJ_H_

#include <cinttypes>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct SharedVariant;
struct SharedVariantStats;
struct ObjectData;
struct StringData;
struct Object;

//////////////////////////////////////////////////////////////////////

/*
 * Representation of an object stored in APC.
 */
struct ImmutableObj {
  explicit ImmutableObj(ObjectData* obj);
  ~ImmutableObj();

  ImmutableObj(const ImmutableObj&) = delete;
  ImmutableObj& operator=(const ImmutableObj&) = delete;

  Object getObject() const;
  void getSizeStats(SharedVariantStats* stats) const;
  int32_t getSpaceUsage() const;

private:
  struct Prop {
    StringData* name;
    SharedVariant* val;
  };

private:
  Prop* m_props;
  int m_propCount;
  StringData* const m_cls;  // static string
};

//////////////////////////////////////////////////////////////////////

}

#endif
