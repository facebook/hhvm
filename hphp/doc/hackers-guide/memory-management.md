## HHVM Memory Layout and Management

This page contains a bird's eye view of HHVM memory layout and management. WIP

## Stack

### C++ stack

The normal function call stack

### VM stack

Contains a mix of TypedValues and Activation Records. `Vmsp` points to this
Both grow down (high address -> low address) and shrink upwards in parallel as the request runs

The OS thread stack is manipulated by compiled C++ as it runs.  The VM thread stack is manipulated by the interpreter or compiled JIT code as it runs.  When JIT code or the interpreter call into C++, they use the OS stack.

If you're running the interpreter (no jit) then the vm stack grows/shrinks with PHP call but you stay in a single C++ activation frame the whole time.

## HHVM Request Heap

All refcounted objects go to heap, all inherit from HeapObject which keeps the refcount. If the refcount goes to 0, the object is freed immediately by pushing it on a freelist (see MemoryManager)

Periodically a mark-sweep garbage collector marks transitive objects starting from roots (RDS, C++ stack, and the VM stack), and frees objects not reachable from a root, even if they have a nonzero refcount. If there were no refcounting bugs, this would only identify objects in or reachable from reference cycles. But with refcounting bugs (missing decrefs), it also cleans up objects whose refcount is incorrect.

The JIT uses profiling to identify decref sites that never reach 0 or 1, and can optimize away the decref. This leads to floating garbage that only the mark-sweep GC can reclaim. It was a small win at some point in the past.

RDS (see below) and the 2 stacks are the root set of the heap.

### Two special types of heap objects

#### Uncounted

Live longer than a request, managed by other mechanisms, note that even uncounted objects have a sentinel refcount but it's used by garbage collector whose value marks the object as uncounted. The refcount primitives use the sentinel to avoid mutating the count of objects shared between requests. The GC uses memory ranges to identify request heap objects without inspecting their refcount

#### Static

Never freed once created. In lowptr builds these are allocated in the low 4GB so we can use 32-bit pointers - see LowPtr<T>.

Together, both of the above go by KindOfPersistent data type.

## RDS

Special, fixed sized area for per request data with request lifetime, addressable consistently by any request.
RDS::link stores the index of each item in the process memory, while a request is executing
vmtl (or R12 in jit'd code) points to the request's RDS segment.
RDS consists of a header, a normal segment, a local segment (persistent but non shared) and a persistent segment (shared persistent)
Read more: hphp/runtime/base/rds.h


## APC

Key value store that sits outside the process memory. Keys are string, values can be {string,object,array}. Objects are stored in a serialized representation while array etc in their native form. Values in APC can be deleted or expire. Treadmill is used to reclaim memory.

APC is implemented as a tbb::ConcurrentHashMap, containing pointers to uncounted objects that live it's all in the hhvm process heap (but not individual request heaps).


## Memory Management

### Reference counting

HHVM's user heap is managed primarily with reference counting. All types that
represent a user-visible value inherit from
[`HeapObject`](../../runtime/base/header-kind.h), which contains the value's
reference count, a `HeaderKind` that describes its type, some mark bits for use
by the garbage collector, and some padding bits available for type-specific
uses. `HeapObject`s are created with a reference count of 1, and this count is
incremented or decremented as references to it are created or destroyed.
When a `HeapObject`'s reference count goes to 0, it is destroyed.


### Garbage collection

HHVM also has a [mark-sweep garbage
collector](https://en.wikipedia.org/wiki/Tracing_garbage_collection) that runs
as a backup for reference counting. `HeapObject`s will be freed by the garbage
collector if they are not reachable from the request's roots, for example when
the only references to them are part of a reference cycle. The collector
primarily lives in [heap-collect.cpp](../../runtime/base/heap-collect.cpp) and
[heap-scan.h](../../runtime/base/heap-scan.h), with [scanning
code](https://github.com/facebook/hhvm/search?q=TYPE_SCAN_CUSTOM&unscoped_q=TYPE_SCAN_CUSTOM)
spread throughout various other files, situated with the types being scanned.

For information on annotations, see [type-scan.h](../../util/type-scan.h).
