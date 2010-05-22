/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_UNSAFE_POINTER_H__
#define __HPHP_UNSAFE_POINTER_H__

#include <util/base.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * When we use LinearAllocator to backup memory, there are pointers that are
 * external, and there is no guarantee that these pointers, when restored, will
 * still be valid. Therefore, any pointers that will participate in checkpoint
 * operation need to implement this interface to properly protect/unprotect at
 * startup time and shutdown time, for example, by increment or decrement a
 * reference count.
 */
class UnsafePointer {
public:
  UnsafePointer();
  virtual ~UnsafePointer();

  /**
   * Has a checkpoint taken? If so, I don't have to protect myself to become
   * persistent. Otherwise, I need to put myself into the set of unsafe
   * pointers which will become persistent at checkpoint time.
   */
  bool beforeCheckpoint();

  /**
   * Protect the object to be persistent.
   */
  virtual void protect() = 0;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_UNSAFE_POINTER_H__
