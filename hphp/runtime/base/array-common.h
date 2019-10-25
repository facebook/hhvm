/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef incl_HPHP_ARRAY_COMMON_H_
#define incl_HPHP_ARRAY_COMMON_H_

#include <sys/types.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct ArrayData;
struct ObjectData;
struct Variant;

//////////////////////////////////////////////////////////////////////

/*
 * This contains shared stubs used for multiple purposes in the
 * ArrayData vtable.  Most of the functions used by more than one
 * kind, or for more than one purpose should be collected here.
 *
 * Note: if you have entry points on an array kind that should never
 * be called, it's generaelly preferable to give them their own unique
 * functions so it will be obvious which kind was involved in core
 * files.  We only try to consolidate the common array functions that
 * should actually be called.
 */
struct ArrayCommon {
  static ssize_t ReturnInvalidIndex(const ArrayData*);

  /*
   * Generic Pop and Dequeue implementations in terms of other functions.
   */
  static ArrayData* Pop(ArrayData*, Variant&);
  static ArrayData* Dequeue(ArrayData*, Variant&);

  static ArrayData* ToVec(ArrayData*, bool);
  static ArrayData* ToDict(ArrayData*, bool);
  static ArrayData* ToKeyset(ArrayData*, bool);

  static ArrayData* ToVArray(ArrayData*, bool);
  static ArrayData* ToDArray(ArrayData*, bool);
};

//////////////////////////////////////////////////////////////////////

ArrayData* castObjToVec(ObjectData*);
ArrayData* castObjToDict(ObjectData*);
ArrayData* castObjToKeyset(ObjectData*);
ArrayData* castObjToVArray(ObjectData*);
ArrayData* castObjToDArray(ObjectData*);

//////////////////////////////////////////////////////////////////////

}

#endif
