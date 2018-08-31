## Memory management

### Reference counting

HHVM's user heap is managed primarily with reference counting. All types that
represent a user-visible value inherit from
[`HeapObject`](../../runtime/base/header-kind.h), which contains the value's
reference count, a `HeaderKind` that describes its type, some mark bits for use
by the garbage collector, and some padding bits available for type-specific
uses. `HeapObject`s are created with a reference count of 1, and this count is
incremented or decremented as references to it are created or destroyed.
When a `HeapObject`'s reference count goes to 0, it is destroyed.

Two reference count values are special:
[`Uncounted`](https://github.com/facebook/hhvm/blob/HHVM-3.27/hphp/runtime/base/header-kind.h#L121)
values live longer than a single Hack request, and have their lifetime managed
by other mechanisms.
[`Static`](https://github.com/facebook/hhvm/blob/HHVM-3.27/hphp/runtime/base/header-kind.h#L122)
values are never freed once created. Together, these are called *persistent*
values, which is where the `KindOfPersistent*` `DataType` names come from.
Persistent values are allocated with `malloc()` and cannot refer to counted
values, which live in request-specific heaps and are allocated by HHVM's request
[memory manager](../../runtime/base/memory-manager.h).

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
