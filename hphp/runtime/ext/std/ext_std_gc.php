<?hh

/* Returns status of the circular reference collector.
 */
<<__Native>>
function gc_enabled(): bool;

/* Activates the circular reference collector.
 */
<<__Native>>
function gc_enable(): void;

/* Deactivates the circular reference collector.
 */
<<__Native>>
function gc_disable(): void;

/* Forces collection of any existing garbage cycles.
 */
<<__Native>>
function gc_collect_cycles()[leak_safe]: int;

/* Reclaims memory used by the memory manager using flush_thread_caches().
 */
<<__Native>>
function gc_mem_caches(): int;

/* Check heap integrity
 */
<<__Native>>
function gc_check_heap(): void;
