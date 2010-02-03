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

#include "shared_memory_allocator.h"
#include "exception.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char *SharedMemoryManager::Name = "HPHPSharedMemory";
boost::interprocess::managed_shared_memory *
SharedMemoryManager::Segment = NULL;

void SharedMemoryManager::Init(int size, bool clean) {
  try {
    if (Segment == NULL) {
      if (clean) {
        boost::interprocess::shared_memory_object::remove(Name);
      }
      Segment = new boost::interprocess::managed_shared_memory
        (boost::interprocess::open_or_create, Name, size);
    }
  } catch (std::exception &e) {
    throw Exception(e.what()); // so we have stacktrace
  }
}

///////////////////////////////////////////////////////////////////////////////
}
