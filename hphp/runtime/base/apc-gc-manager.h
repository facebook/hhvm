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
#ifndef incl_HPHP_APC_MEMORY_MANAGER_H_
#define incl_HPHP_APC_MEMORY_MANAGER_H_

#include <map>
#include <set>
#include <atomic>

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/util/lock.h"
#include "hphp/util/mutex.h"
#include "hphp/util/trace.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////
/*
 * GC manager for all to-be-deleted APC * UNCOUNTED * values.
 * A value is to-be-deleted if there is no reference pointing to it.
 * It's a thread-safe class. There's only 1 instance for all threads.
 *
 * APCTypedValue::registerUncountedAllocations() should recursively traverse
 * the data and call APCGCManager::registerAllocation(st, ed, root)
 * to records all the visited allocations [st,ed).
 * root is the APCHandle* of the top-level data. It should NOT be contained in
 * any other array.
 * For example, if $a = ["str1",$b], $b = ["str2"]
 * There will be 4 APCGCManager::registerAllocation(st, ed, root) calls:
 * 1. { [StringData,ArrayData], $a }
 * 2. { "str1", $a }
 * 3. { [StringData], $a }
 * 4. { "str2", $a }
 * To keep it simple, we won't do partially free on an APC uncounted -- either
 * don't free any allocation, or free whole uncounted.
 *
 * To support GC for APC, Manager needs a mapping:
 * {root -> visibleFlag}.
 * When the apcgcPendingSize exceeded a bar, invoke a global GC for all threads.
 * After glocal GC ended, The true visbile flag shows there is a heap object
 * referecing to APC uncounted data(The root handle).
 * For each root, We can free it if visbileFlag=false
 *
 * Set -Eval.GCForAPC=true to use it. It won't do anything in server mode, but
 * will do GC on APC in script mode.
 * Following things will be changed with -Eval.GCForAPC=true:
 * 1. Heap collector will call APCGCManager::sweep() during Marker::sweep(),
 * call APCGCManager::mark() during Marker::checkedEnqueue()
 * 2. Treadmill will call APCGCManager::registerPendingDeletion(handle, size)
 * when enqueue a deletion. This function will register all related allocations
 * by traversing the data recursively.
 * 3. Treadmill will call APCGCManager::freeAPCHandles() to free all pending
 * deletion during Treamill round instead of free them itself.
 *
 * Without -Eval.GCForAPC=true, everything is as same as APCGCManager
 * never exists
 *
 * TODO Task #20074509: API for global GC invoke
*/

struct APCGCManager {
  public:
    using Allocation = std::pair<const void*, const void*>;
    // Register {allocation, root}
    void registerAllocation(void* start, void* end, APCHandle* root);
    // Increase pending size for APC uncounted data, register this APCHandle*
    // And rigster all allocations belong to this root by recursively traversing
    // the data
    void registerPendingDeletion(APCHandle* root, const size_t size);
    // Loop all to-be-deleted allocations, pull out their root
    // If the root is invisible, free it
    void sweep();
    // Mark the root to be visible for this ptr
    bool mark(const void* ptr);
    // Treadmill ask APCGCManager to free all those handles if -Eval.GCForAPC
    // =true.
    // When running as server mode, free those handles immediately, just
    // like what Treadmill did
    // When running not as server mode, free handles if we haven't free
    // them before
    void freeAPCHandles(const std::vector<APCHandle*>& handles);
    // Size of to-be-deleted APC uncounted data
    size_t apcgcPendingSize() {
      assertx(RuntimeOption::EvalGCForAPC);
      return pendingSize.load();
    }
    // To be global unique
    static APCGCManager& getInstance();

  private:
    void invokeGlobalGC();
    void insert(Allocation r, APCHandle* root);
    APCHandle* getRootAPCHandle(const void* ptr);
    void insertVisibleRoot(APCHandle *root);

  private:
    // Size of total to-be-deleted APC uncounted data
    std::atomic<size_t> pendingSize;
    // Mapping { (stAddress, edAddress) -> root } }
    std::map<Allocation, APCHandle*> rootMap;
    ReadWriteMutex rootMapLock;
    // Every root in this set is visible from heap
    std::set<APCHandle*> visibleFromHeap;
    ReadWriteMutex visibleFromHeapLock;
    // To ensure won't free same handle twice
    // We can free an APCHandle* if and only if it is contained by this list
    // It also shows the size of the whole APC uncounted value
    std::map<APCHandle*, size_t> candidateList;
    ReadWriteMutex candidateListLock;
    // TODO decide policy
    // bar for doing GC for APC
    // using runtime flag -vEval.GCForAPCTrigger
    size_t someBar() {
      return RuntimeOption::EvalGCForAPCTrigger;
    }
};
//////////////////////////////////////////////////////////////////////

}

#endif
