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

#ifndef __HPHP_LINEAR_ALLOCATOR_H__
#define __HPHP_LINEAR_ALLOCATOR_H__

#include <util/base.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A LinearAllocator linearizes or serializes any number of runtime objects'
 * internally malloc-ed and variable sized memory into one single continuous
 * piece. This allows us to backup and restore runtime object's states quickly.
 * Combined with fixed size SmartAllocator, we can then do generational
 * sweeping to solve circular reference problem of reference counting, we
 * can do checkpoints of the entire user memory space, and we can initializes
 * static arrays a lot faster.
 */
class LinearAllocator {
public:
  LinearAllocator();
  ~LinearAllocator();
  void reset();

  /**
   * Backup functions.
   */
  void beginBackup(int size, int count);
  void backup(void *p);
  void backup(int size);
  void backup(const char *data, int size);
  void endBackup();

  /**
   * Restore functions.
   */
  void beginRestore();
  void *restore(const char *&data);
  void advance(int linearSize, int linearCount) {
    m_pos += linearSize;
    m_index += linearCount;
  }
  void endRestore();

  void checkMemory(bool detailed);

private:
  char *m_blob;
  int m_size;
  int m_pos;

  struct ItemInfo {
    void *p;
    int end_pos;
  };

  std::vector<ItemInfo> m_items;
  int m_count;
  int m_index;
  ItemInfo *m_item;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_LINEAR_ALLOCATOR_H__
