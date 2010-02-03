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

#ifndef __MEMORY_SHARED_MEMORY_ALLOCATOR_H__
#define __MEMORY_SHARED_MEMORY_ALLOCATOR_H__

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include "base.h"

namespace HPHP {

/**
 * HOW TO USE SHARED MEMORY FOR MY OBJECTS?
 * ========================================
 *
 * 1. Make sure your class is POD (plain-old data object), simply meaning
 *    there cannot be virtual functions in the class. Derivation is okay, but
 *    just not virtual functions, because virtual function table has address
 *    pointers that are always local to creation process, and those are not
 *    pass-able between different processes. If you have it, the symptom would
 *    be, well it works for the creation process, but it will fail miserably
 *    when a 2nd process attaches to the same shared memory and call some
 *    functions (or other things) that use vtable.
 *
 * 2. Make sure ALL data members, no matter how deep it is, are shared memory
 *    safe. Primitive types are automatically shared memory safe, thanks to the
 *    allocator implemented in this file. For complex types, we just need to
 *    make sure all STL classes are converted to use their counterparts:
 *
 *      std::string => SharedMemoryString
 *      std::vector => SharedMemoryVector
 *      std::list   => SharedMemoryList
 *      std::set    => SharedMemorySet
 *      std::map    => SharedMemoryMap
 *
 *    There is a macro DISABLE_SHARED_MEMORY for debugging purpose, turning off
 *    this conversion transparently.
 *
 *    For any other complex types that are not covered here, they have to have
 *    a way to specify its memory allocator to use the allocator in this file.
 *    This normally means dirty work redirecting all malloc/free to call
 *    methods on the allocator and pass this allocator into the class. So it
 *    may not be trivial.
 *
 * 3. During application startup time, call this to initialize/attach to an
 *    existing piece of shared memory:
 *
 *      SharedMemoryManager::Init(totalSizeInBytes);
 *
 *    Shared memory can actually resizable, but it's not implemented in this
 *    file yet. Look up this web site for adding features in this file:
 *
 *      http://www.boost.org/doc/libs/1_35_0/doc/html/interprocess.html
 *
 * 4. Now, we need a top level object to pull out anything from shared memory,
 *    right? This kind of top level objects always have a name to uniquely
 *    identify them:
 *
 *       MyClass *top_level_object =
 *         SharedMemory<MyClass>::OpenOrCreate("a unique name for my object");
 *
 *    Note SharedMemory<T> works just fine with primitive types:
 *
 *       int *secret_integer = SharedMemory<int>::OpenOrCreate("secret");
 *
 * 5. If we are the sole owner of a shared memory object, we don't need locking
 *    at all. If not, we need to lock individual objects for accesses:
 *
 *       int *shared = SharedMemory<int>::OpenOrCreate("shared");
 *       {
 *         SharedMemoryLock lock("any name for this lock");
 *         *shared = whatever;
 *       }
 *
 *    Two things to notice, (1) SharedMemory<T>::OpenOrCreate() is atomic
 *    already, and there is no need to lock protect that. It also made sure
 *    same key/name never gets created twice by two concurrent processes.
 *    (2) lock's name has NOTHING to do with the object's name. So you can use
 *    the same lock for multiple objects. Or, more often than not, you can use
 *    multple locks for sub-component of the same object -- finer locking.
 *
 *    It's a dumb simple mutex-based lock right now. It's possible to have
 *    read-write locks as well, just not implemented yet.
 */

///////////////////////////////////////////////////////////////////////////////

/**
 * Helper class to hold a segment of shared-memory. We only need one instance
 * of this class for one application for simplicity.
 */
class SharedMemoryManager {
public:
  // an app needs to call this during startup time
  static void Init(int size, bool clean);

  // called by allocator class
  static boost::interprocess::managed_shared_memory *GetSegment() {
    ASSERT(Segment); // or Init() is missing
    return Segment;
  }

private:
  static const char *Name;
  static boost::interprocess::managed_shared_memory *Segment;
};

/**
 * Helper class for STL classes to use our SharedMemoryManager for allocating
 * elements.
 */
template<typename T>
class SharedMemoryAllocator : public boost::interprocess::allocator
<T, boost::interprocess::managed_shared_memory::segment_manager> {
public:
  SharedMemoryAllocator() :
    boost::interprocess::allocator
  <T, boost::interprocess::managed_shared_memory::segment_manager>
  (SharedMemoryManager::GetSegment()->get_segment_manager()) {
  }
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Creating or destroying a top-level shared memory objects with names.
 */
template<typename T>
class SharedMemory {
public:
  static T *OpenOrCreate(const char *name) {
    ASSERT(name && *name);
    return SharedMemoryManager::GetSegment()->find_or_construct<T>(name)();
  }
  static bool Destroy(const char *name) {
    ASSERT(name && *name);
    return SharedMemoryManager::GetSegment()->destroy<T>(name);
  }
};

///////////////////////////////////////////////////////////////////////////////

#ifndef DISABLE_SHARED_MEMORY

/**
 * Embeddable STL classes: string, vector, list, set, map.
 */
typedef boost::interprocess::string SharedMemoryString;

// This sucks, since C++ doesn't support partial template typedefs.

template<typename T>
class SharedMemoryVector :
    public boost::interprocess::vector<T, SharedMemoryAllocator<T> > {
};
template<typename T>
class SharedMemoryList :
    public boost::interprocess::list<T, SharedMemoryAllocator<T> > {
};
template<typename T>
class SharedMemorySet :
    public boost::interprocess::set<T, std::less<T>,
                                    SharedMemoryAllocator<T> > {
};
template<typename Key, typename T>
class SharedMemoryMap :
    public boost::interprocess::map
    <Key, T, std::less<Key>, SharedMemoryAllocator<std::pair<Key, T> > > {
};

template<typename Key, typename T, class C>
class SharedMemoryMapWithComp :
    public boost::interprocess::map
    <Key, T, C, SharedMemoryAllocator<std::pair<Key, T> > > {
};

#else

// falling back to regular std classes
typedef std::string SharedMemoryString;
template<typename T> class SharedMemoryVector : public std::vector<T> {};
template<typename T> class SharedMemoryList : public std::list<T> {};
template<typename T> class SharedMemorySet : public std::set<T> {};
template<typename K, typename T>
class SharedMemoryMap : public std::map<K, T> {};

#endif

///////////////////////////////////////////////////////////////////////////////

/**
 * Lock/unlock a top level shared memory object for sole access.
 */
class SharedMemoryLock {
public:
  static bool Destroy(const char *name) {
    ASSERT(name && *name);
    return boost::interprocess::named_mutex::remove(name);
  }

  SharedMemoryLock(const char *name) {
    ASSERT(name && *name);
    m_mutex = new boost::interprocess::named_mutex
      (boost::interprocess::open_or_create, name);
    m_mutex->lock();
  }

  ~SharedMemoryLock() {
    m_mutex->unlock();
    delete m_mutex;
  }

private:
  boost::interprocess::named_mutex *m_mutex;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __MEMORY_SHARED_MEMORY_ALLOCATOR_H__
