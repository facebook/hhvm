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
function gc_collect_cycles(): int;
