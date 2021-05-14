<?hh // partial

/**
 * Caches a variable in the data store, only if it's not already stored.
 *   Unlike many other mechanisms in PHP, variables stored using apc_add() will
 *   persist between requests (until the value is removed from the cache).
 *
 * @param mixed $key_or_array - Store the variable using this name. Can also
 *   be an array of key-value pairs to insert into the cache. Keys are
 *   cache-unique, so attempting to use apc_add() to store data with a key that
 *   already exists will not overwrite the existing data, and will instead
 *   return FALSE. (This is the only difference between apc_add() and
 *   apc_store().)
 * @param mixed $var - The variable to store. Defaults to null for the case
 *   where an array of key-value pairs is passed.
 * @param int $ttl - Time To Live; store var in the cache for ttl seconds.
 *   After the ttl has passed, the stored variable will be expunged from the
 *   cache (on the next request). If no ttl is supplied (or if the ttl is 0),
 *   the value will persist until it is removed from the cache manually, or
 *   otherwise fails to exist in the cache (clear, restart, etc.).
 *
 * @return mixed - Returns TRUE on success or FALSE on failure and an array
 *   with error keys if passed an array.
 *
 */
<<__Native, __NonRx('APC')>>
function apc_add(mixed $key_or_array,
                 mixed $var = null,
                 int $ttl = 0,
                 int $bump_ttl = 0): mixed;

/**
 * Cache a variable in the data store. Unlike many other mechanisms in PHP,
 *   variables stored using apc_store() will persist between requests (until the
 *   value is removed from the cache).
 *
 * @param mixed $key_or_array - Store the variable using this name. Can also
 *   be an array of key-value pairs to insert into the cache. Keys are
 *   cache-unique, so storing a second value with the same key will overwrite
 *   the original value.
 * @param mixed $var - The variable to store. Defaults to null for the case
 *   where an array of key-value pairs is passed.
 * @param int $ttl - Time To Live; store var in the cache for ttl seconds.
 *   After the ttl has passed, the stored variable will be expunged from the
 *   cache (on the next request). If no ttl is supplied (or if the ttl is 0),
 *   the value will persist until it is removed from the cache manually, or
 *   otherwise fails to exist in the cache (clear, restart, etc.).
 *
 * @return mixed - Returns TRUE on success, FALSE on failure and an array with
 *   error keys if passed an array.
 *
 */
<<__Native, __NonRx('APC')>>
function apc_store(mixed $key_or_array,
                   mixed $var = null,
                   int $ttl = 0,
                   int $bump_ttl = 0): mixed;

/**
 * Fetches a stored variable from the cache.
 *
 * @param mixed $key - The key used to store the value (with apc_store()). If
 *   an array is passed then each element is fetched and returned.
 * @param mixed $success - Set to TRUE in success and FALSE in failure.
 *
 * @return mixed - The stored variable or array of variables on success; FALSE
 *   on failure
 *
 */
<<__Native, __NonRx('APC')>>
function apc_fetch(mixed $key,
                   <<__OutOnly("KindOfBoolean")>> inout mixed $success): mixed;

/**
 * Removes a stored variable from the cache.
 *
 * @param mixed $key - The key used to store the value (with apc_store()).
 *
 * @return mixed - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native, __NonRx('APC')>>
function apc_delete(mixed $key): mixed;

/**
 * Retrieves cached information and meta-data from APC's data store.
 *
 * @param string $cache_type - If cache_type is "user", information about the
 *   user cache will be returned.  If cache_type is "filehits", information
 *   about which files have been served from the bytecode cache for the current
 *   request will be returned. This feature must be enabled at compile time
 *   using --enable-filehits .  If an invalid or no cache_type is specified,
 *   information about the system cache (cached files) will be returned.
 * @param bool $limited - If limited is TRUE, the return value will exclude
 *   the individual list of cache entries. This is useful when trying to
 *   optimize calls for statistics gathering.
 *
 * @return mixed - Array of cached data (and meta-data) or FALSE on failure
 *   apc_cache_info() will raise a warning if it is unable to retrieve APC cache
 *   data. This typically occurs when APC is not enabled.
 *
 */
<<__Native, __NonRx('APC')>>
function apc_cache_info(string $cache_type = "", bool $limited = false): darray;

/**
 * Clears the user/system cache.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native, __NonRx('APC')>>
function apc_clear_cache(string $cache_type = ""): bool;

/**
 * Retrieves APC's Shared Memory Allocation information.
 *
 * @param bool $limited - When set to FALSE (default) apc_sma_info() will
 *   return a detailed information about each segment.
 *
 * @return array - Array of Shared Memory Allocation data; FALSE on failure.
 *
 */
<<__NonRx('APC')>>
function apc_sma_info(bool $limited = false): darray { return darray[]; }

/**
 * Increases a stored number.
 *
 * @param string $key - The key of the value being increased.
 * @param int $step - The step, or value to increase.
 * @param mixed $success - Optionally pass the success or fail boolean value
 *   to this referenced variable.
 *
 * @return mixed - Returns the current value of key's value on success, or
 *   FALSE on failure
 *
 */
<<__Native, __NonRx('APC')>>
function apc_inc(string $key,
                 int $step,
                 <<__OutOnly("KindOfBoolean")>>
                 inout mixed $success): mixed;

/**
 * Decreases a stored integer value.
 *
 * @param string $key - The key of the value being decreased.
 * @param int $step - The step, or value to decrease.
 * @param mixed $success - Optionally pass the success or fail boolean value
 *   to this referenced variable.
 *
 * @return mixed - Returns the current value of key's value on success, or
 *   FALSE on failure
 *
 */
<<__Native, __NonRx('APC')>>
function apc_dec(string $key,
                 int $step,
                 <<__OutOnly("KindOfBoolean")>>
                 inout mixed $success): mixed;

/**
 * Update an existing old value to a new value.
 *
 * @param string $key - The key of the value being updated.
 * @param int $old_cas - The old value that is currently stored.
 * @param int $new_cas - The new value to update to.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native, __NonRx('APC')>>
function apc_cas(string $key,
                 int $old_cas,
                 int $new_cas): bool;

/**
 * Checks if one ore more APC keys exist.
 *
 * @param mixed $key - The key to check existence. If an array is passed then
 *   each element is checked.
 *
 * @return mixed - TRUE if the key exists, otherwise FALSE. If array is passed
 *   in, then an array is returned that contains all existing keys, or an empty
 *   array if none exist.
 *
 */
<<__Native, __NonRx('APC')>>
function apc_exists(mixed $key): mixed;

/**
 * Extend the TTL of a key in APC to now + a new ttl (or infinite). If the
 * effective expiration of the key is longer than this, returns FALSE and
 * does nothing.
 *
 * @param string $key - The key to bump. If the key is not stored in APC, then
 *   FALSE is returned.
 *
 * @param int $new_ttl - The new TTL for the key. 0 means infinite TTL.
 *
 * @return bool - TRUE if the TTL was actually extended, FALSE otherwise.
 */
 <<__Native, __NonRx('APC')>>
 function apc_extend_ttl(string $key, int $new_ttl): bool;

/**
 * Find the in-memory size of a key in APC, for debugging purposes.
 *
 * @param string $key - The key to find the size of.
 *
 * @return mixed - Returns the current size of a key or null on failure.
 *
 */
<<__Native, __NonRx('APC')>>
function apc_size(string $key): ?int;
