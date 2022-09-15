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

#ifndef __TBB_tbb_H
#define __TBB_tbb_H

/**
    This header bulk-includes declarations or definitions of all the functionality
    provided by TBB (save for malloc dependent headers).

    If you use only a few TBB constructs, consider including specific headers only.
    Any header listed below can be included independently of others.
**/

#if TBB_PREVIEW_AGGREGATOR
#include "aggregator.h"
#endif
#include "aligned_space.h"
#include "atomic.h"
#include "blocked_range.h"
#include "blocked_range2d.h"
#include "blocked_range3d.h"
#include "cache_aligned_allocator.h"
#include "combinable.h"
#include "concurrent_hash_map.h"
#if TBB_PREVIEW_CONCURRENT_LRU_CACHE
#include "concurrent_lru_cache.h"
#endif
#include "concurrent_priority_queue.h"
#include "concurrent_queue.h"
#include "concurrent_unordered_map.h"
#include "concurrent_unordered_set.h"
#include "concurrent_vector.h"
#include "critical_section.h"
#include "enumerable_thread_specific.h"
#include "flow_graph.h"
#if TBB_PREVIEW_GLOBAL_CONTROL
#include "global_control.h"
#endif
#include "mutex.h"
#include "null_mutex.h"
#include "null_rw_mutex.h"
#include "parallel_do.h"
#include "parallel_for.h"
#include "parallel_for_each.h"
#include "parallel_invoke.h"
#include "parallel_reduce.h"
#include "parallel_scan.h"
#include "parallel_sort.h"
#include "partitioner.h"
#include "pipeline.h"
#include "queuing_mutex.h"
#include "queuing_rw_mutex.h"
#include "reader_writer_lock.h"
#include "recursive_mutex.h"
#include "spin_mutex.h"
#include "spin_rw_mutex.h"
#include "task.h"
#include "task_arena.h"
#include "task_group.h"
#include "task_scheduler_init.h"
#include "task_scheduler_observer.h"
#include "tbb_allocator.h"
#include "tbb_exception.h"
#include "tbb_thread.h"
#include "tick_count.h"

#endif /* __TBB_tbb_H */
