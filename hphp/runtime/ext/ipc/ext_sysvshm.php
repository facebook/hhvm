<?hh

/**
 * shm_attach() returns an id that can be used to access the System V shared
 *   memory with the given key, the first call creates the shared memory segment
 *   with memsize and the optional perm-bits perm.  A second call to
 *   shm_attach() for the same key will return a different shared memory
 *   identifier, but both identifiers access the same underlying shared memory.
 *   memsize and perm will be ignored.
 *
 * @param int $shm_key - A numeric shared memory segment ID
 * @param int $shm_size - The memory size. If not provided, default to the
 *   sysvshm.init_mem in the php.ini, otherwise 10000 bytes.
 * @param int $shm_flag - The optional permission bits. Default to 0666.
 *
 * @return mixed - Returns a shared memory segment identifier.
 *
 */
<<__Native>>
function shm_attach(int $shm_key,
                    int $shm_size = 10000,
                    int $shm_flag = 0666): mixed;

/**
 * shm_detach() disconnects from the shared memory given by the shm_identifier
 *   created by shm_attach(). Remember, that shared memory still exist in the
 *   Unix system and the data is still present.
 *
 * @param int $shm_identifier - A shared memory resource handle as returned by
 *   shm_attach()
 *
 * @return bool - shm_detach() always returns TRUE.
 *
 */
<<__Native>>
function shm_detach(int $shm_identifier): bool;

/**
 * shm_remove() removes the shared memory shm_identifier. All data will be
 *   destroyed.
 *
 * @param int $shm_identifier - The shared memory identifier as returned by
 *   shm_attach()
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function shm_remove(int $shm_identifier): bool;

/**
 * shm_get_var() returns the variable with a given variable_key, in the given
 *   shared memory segment. The variable is still present in the shared memory.
 *
 * @param int $shm_identifier - Shared memory segment, obtained from
 *   shm_attach().
 * @param int $variable_key - The variable key.
 *
 * @return mixed - Returns the variable with the given key.
 *
 */
<<__Native>>
function shm_get_var(int $shm_identifier, int $variable_key): mixed;

/**
 * shm_has_var() checks whether a specific key exists inside a shared memory
 *   segment.
 *
 * @param int $shm_identifier - Shared memory segment, obtained from
 *   shm_attach().
 * @param int $variable_key - The variable key.
 *
 * @return bool - Returns TRUE if the entry exists, otherwise FALSE
 *
 */
<<__Native>>
function shm_has_var(int $shm_identifier, int $variable_key): bool;

/**
 * shm_put_var() inserts or updates the variable with the given variable_key.
 *   Warnings (E_WARNING level) will be issued if shm_identifier is not a valid
 *   SysV shared memory index or if there was not enough shared memory remaining
 *   to complete your request.
 *
 * @param int $shm_identifier - A shared memory resource handle as returned by
 *   shm_attach()
 * @param int $variable_key - The variable key.
 * @param mixed $variable - The variable. All variable-types are supported.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function shm_put_var(int $shm_identifier,
                     int $variable_key,
                     mixed $variable): bool;

/**
 * Removes a variable with a given variable_key and frees the occupied memory.
 *
 * @param int $shm_identifier - The shared memory identifier as returned by
 *   shm_attach()
 * @param int $variable_key - The variable key.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function shm_remove_var(int $shm_identifier, int $variable_key): bool;
