/*
    Copyright (c) 2005-2018 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.




*/

#ifndef _itt_shared_malloc_MapMemory_H
#define _itt_shared_malloc_MapMemory_H

#include <stdlib.h>

void *ErrnoPreservingMalloc(size_t bytes)
{
    int prevErrno = errno;
    void *ret = malloc( bytes );
    if (!ret)
        errno = prevErrno;
    return ret;
}

#if __linux__ || __APPLE__ || __sun || __FreeBSD__

#if __sun && !defined(_XPG4_2)
 // To have void* as mmap's 1st argument
 #define _XPG4_2 1
 #define XPG4_WAS_DEFINED 1
#endif

#include <sys/mman.h>
#if __linux__
/* __TBB_MAP_HUGETLB is MAP_HUGETLB from system header linux/mman.h.
   The header is not included here, as on some Linux flavors inclusion of
   linux/mman.h leads to compilation error,
   while changing of MAP_HUGETLB is highly unexpected.
*/
#define __TBB_MAP_HUGETLB 0x40000
#else
#define __TBB_MAP_HUGETLB 0
#endif

#if XPG4_WAS_DEFINED
 #undef _XPG4_2
 #undef XPG4_WAS_DEFINED
#endif

inline void* mmap_impl(size_t map_size, void* map_hint = NULL, int map_flags = 0) {
#ifndef MAP_ANONYMOUS
// macOS* defines MAP_ANON, which is deprecated in Linux*.
#define MAP_ANONYMOUS MAP_ANON
#endif /* MAP_ANONYMOUS */
    return mmap(map_hint, map_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | map_flags, -1, 0);
}

inline void* mmapTHP(size_t bytes) {
    // Initializes in zero-initialized data section
    static void* hint;

    // Optimistically try to use a last huge page aligned region end
    // as a hint for mmap.
    hint = hint ? (void*)((uintptr_t)hint - bytes) : hint;
    void* result = mmap_impl(bytes, hint);

    // Something went wrong
    if (result == MAP_FAILED) {
        hint = NULL;
        return MAP_FAILED;
    }

    // Otherwise, fall back to the slow path - map oversized region
    // and trim excess parts.
    if (!isAligned(result, HUGE_PAGE_SIZE)) {
        // Undo previous try
        munmap(result, bytes);

        // Map oversized on huge page size region
        result = mmap_impl(bytes + HUGE_PAGE_SIZE);

        // Something went wrong
        if (result == MAP_FAILED) {
            hint = NULL;
            return MAP_FAILED;
        }

        // Misalignment offset
        uintptr_t offset = 0;

        if (!isAligned(result, HUGE_PAGE_SIZE)) {
            // Trim excess head of a region if it is no aligned
            offset = HUGE_PAGE_SIZE - ((uintptr_t)result & (HUGE_PAGE_SIZE - 1));
            munmap(result, offset);

            // New region begining
            result = (void*)((uintptr_t)result + offset);
        }

        // Trim excess tail of a region
        munmap((void*)((uintptr_t)result + bytes), HUGE_PAGE_SIZE - offset);
    }

    // Assume, that mmap virtual addresses grow down by default
    // So, set a hint as a result of a last successfull allocation
    // and then use it minus requested size as a new mapping point.
    // TODO: Atomic store is meant here, fence not needed, but
    // currently we don't have such function.
    hint = result;

    MALLOC_ASSERT(isAligned(result, HUGE_PAGE_SIZE), "Mapped address is not aligned on huge page size.");

    return result;
}

#define MEMORY_MAPPING_USES_MALLOC 0
void* MapMemory (size_t bytes, PageType pageType)
{
    void* result = 0;
    int prevErrno = errno;

    switch (pageType) {
        case REGULAR:
        {
            result = mmap_impl(bytes);
            break;
        }
        case PREALLOCATED_HUGE_PAGE:
        {
            MALLOC_ASSERT((bytes % HUGE_PAGE_SIZE) == 0, "Mapping size should be divisible by huge page size");
            result = mmap_impl(bytes, NULL, __TBB_MAP_HUGETLB);
            break;
        }
        case TRANSPARENT_HUGE_PAGE:
        {
            MALLOC_ASSERT((bytes % HUGE_PAGE_SIZE) == 0, "Mapping size should be divisible by huge page size");
            result = mmapTHP(bytes);
            break;
        }
        default:
        {
            MALLOC_ASSERT(NULL, "Unknown page type");
        }
    }

    if (result == MAP_FAILED) {
        errno = prevErrno;
        return 0;
    }

    return result;
}

int UnmapMemory(void *area, size_t bytes)
{
    int prevErrno = errno;
    int ret = munmap(area, bytes);
    if (-1 == ret)
        errno = prevErrno;
    return ret;
}

#elif (_WIN32 || _WIN64) && !__TBB_WIN8UI_SUPPORT
#include <windows.h>

#define MEMORY_MAPPING_USES_MALLOC 0
void* MapMemory (size_t bytes, PageType)
{
    /* Is VirtualAlloc thread safe? */
    return VirtualAlloc(NULL, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

int UnmapMemory(void *area, size_t /*bytes*/)
{
    BOOL result = VirtualFree(area, 0, MEM_RELEASE);
    return !result;
}

#else

#define MEMORY_MAPPING_USES_MALLOC 1
void* MapMemory (size_t bytes, PageType)
{
    return ErrnoPreservingMalloc( bytes );
}

int UnmapMemory(void *area, size_t /*bytes*/)
{
    free( area );
    return 0;
}

#endif /* OS dependent */

#if MALLOC_CHECK_RECURSION && MEMORY_MAPPING_USES_MALLOC
#error Impossible to protect against malloc recursion when memory mapping uses malloc.
#endif

#endif /* _itt_shared_malloc_MapMemory_H */
