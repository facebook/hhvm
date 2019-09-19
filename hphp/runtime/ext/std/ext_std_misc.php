<?hh // partial

namespace HH {

/**
 * Returns a (bad) description of the warmup status of the server, based on
 * request-specific state.
 *
 * @return string - If the server appears to be warmed up, returns the empty
 * string. Otherwise, returns a human-readable description of why the server is
 * not warmed up. Note that this function checks a series of heuristics rather
 * than anything definitive; returning '' for one request does not guarantee
 * the same result for subsequent requests.
 */
<<__Native>>
function server_warmup_status(): string;

/**
 * Returns a good description of the warmup status of the server, based on
 * process-global state.
 *
 * @return string - If the server appears to be warmed up, returns the empty
 * string. Otherwise, returns a human-readable description of why the server is
 * not warmed up. Unlike server_warmup_status(), this function is monotonic,
 * i.e., once it returns empty string, it will keep returning empty string.
 */
<<__Native>>
function server_warmup_status_monotonic(): string;

/**
 * Returns a description of the context in which the request is executing.
 *
 * @return string - If the request was initiated via the proxygen, xbox,
 * pagelet, fastcgi, or replay servers those values are returned. In client
 * mode the string cli is returned, when executing in client mode on a server
 * (via the unix socket interface) clisrv is returned. On the server with an
 * unknown context the string "worker" is returned indicating the job was run
 * on an unnamed JobQueue within the server.
 */
<<__Native>>
function execution_context(): string;

<<__Native>>
function enable_legacy_behavior(vec_or_dict $v): vec_or_dict;

<<__Native>>
function is_legacy_behavior_enabled(vec_or_dict $v): bool;

/**
 * This function is a kludge that returns the last argument it receives
 */
<<__Native>>
function sequence(mixed ... $args): mixed;

}

namespace {

/** Checks whether the client disconnected.
 * @return int - Returns 1 if client disconnected, 0 otherwise.
 */
<<__Native>>
function connection_aborted(): int;

/** Gets the connection status bitfield.
 * @return int - Returns the connection status bitfield, which can be used
 * against the CONNECTION_XXX constants to determine the connection status.
 */
<<__Native>>
function connection_status(): int;

/** Determines whether the script timed out.
 * @return int - Returns 1 if the script timed out, 0 otherwise.
 */
<<__Native>>
function connection_timeout(): int;

/** @param string $name - The constant name.
 * @return mixed - Returns the value of the constant, or NULL if the constant
 * is not defined.
 */
<<__Native>>
function constant(string $name): mixed;

/** Checks whether the given constant exists and is defined.  If you want to
 * see if a variable exists, use isset() as defined() only applies to
 * constants. If you want to see if a function exists, use function_exists().
 * @param string $name - The constant name.
 * @param bool $autoload - Whether to try to autoload.
 * @return bool - Returns TRUE if the named constant given by name has been
 * defined, FALSE otherwise.
 */
<<__Native, __Rx>>
function defined(string $name,
                 bool $autoload = true): bool;

/** Sets whether a client disconnect should cause a script to be aborted.  When
 * running PHP as a command line script, and the script's tty goes away
 * without the script being terminated then the script will die the next time
 * it tries to write anything, unless value is set to TRUE
 * @param bool $setting - If set, this function will set the ignore_user_abort
 * ini setting to the given value. If not, this function will only return the
 * previous setting without changing it.
 * @return int - Returns the previous setting, as an integer.
 */
<<__Native>>
function ignore_user_abort(bool $setting = false): int;

/** Pack given arguments into binary string according to format.  The idea for
 * this function was taken from Perl and all formatting codes work the same as
 * in Perl. However, there are some formatting codes that are missing such as
 * Perl's "u" format code.  Note that the distinction between signed and
 * unsigned values only affects the function unpack(), where as function
 * pack() gives the same result for signed and unsigned format codes.  Also
 * note that PHP internally stores integer values as signed values of a
 * machine-dependent size. If you give it an unsigned integer value too large
 * to be stored that way it is converted to a float which often yields an
 * undesired result.
 * @param string $format - The format string consists of format codes followed
 * by an optional repeater argument. The repeater argument can be either an
 * integer value or * for repeating to the end of the input data. For a, A, h,
 * H the repeat count specifies how many characters of one data argument are
 * taken, for @ it is the absolute position where to put the next data, for
 * everything else the repeat count specifies how many data arguments are
 * consumed and packed into the resulting binary string.  Currently
 * implemented formats are: pack() format characters Code Description a
 * NUL-padded string A SPACE-padded string h Hex string, low nibble first H
 * Hex string, high nibble first csigned char C unsigned char s signed short
 * (always 16 bit, machine byte order) S unsigned short (always 16 bit,
 * machine byte order) n unsigned short (always 16 bit, big endian byte order)
 * v unsigned short (always 16 bit, little endian byte order) i signed integer
 * (machine dependent size and byte order) I unsigned integer (machine
 * dependent size and byte order) l signed long (always 32 bit, machine byte
 * order) L unsigned long (always 32 bit, machine byte order) N unsigned long
 * (always 32 bit, big endian byte order) V unsigned long (always 32 bit,
 * little endian byte order) f float (machine dependent size and
 * representation) d double (machine dependent size and representation) x NUL
 * byte X Back up one byte @ NUL-fill to absolute position
 * @return mixed - Returns a binary string containing data.
 */
<<__Native, __IsFoldable, __Rx>>
function pack(string $format, ...$args): mixed;

/** @param int $seconds - Halt time in seconds.
 * @return int - Returns zero on success, or FALSE on errors. If the call was
 * interrupted by a signal, sleep() returns the number of seconds left to
 * sleep.
 *
 * FCallBuiltin is not used, as it would optimize away event hooks, resulting
 * in broken request timeout semantics. It's also desirable to make sleep()
 * frames visible in profiling tools such as Xenon.
 */
<<__Native("NoFCallBuiltin")>>
function sleep(int $seconds): int;

/** Delays program execution for the given number of micro seconds.
 * @param int $micro_seconds - Halt time in micro seconds. A micro second is
 * one millionth of a second.
 *
 * See sleep() wrt NoFCallBuiltin.
 */
<<__Native("NoFCallBuiltin")>>
function usleep(int $micro_seconds): void;

/** Delays program execution for the given number of seconds and nanoseconds.
 * @param int $seconds - Must be a positive integer.
 * @param int $nanoseconds - Must be a positive integer less than 1 billion.
 * @return mixed - Returns TRUE on success or FALSE on failure.  If the delay
 * was interrupted by a signal, an associative array will be returned with the
 * components: seconds - number of seconds remaining in the delay nanoseconds
 * - number of nanoseconds remaining in the delay
 *
 * See sleep() wrt NoFCallBuiltin.
 */
<<__Native("NoFCallBuiltin")>>
function time_nanosleep(int $seconds,
                        int $nanoseconds): mixed;

/** Makes the script sleep until the specified timestamp.
 * @param float $timestamp - The timestamp when the script should wake.
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 * See sleep() wrt NoFCallBuiltin.
 */
<<__Native("NoFCallBuiltin")>>
function time_sleep_until(float $timestamp): bool;

/** Gets a prefixed unique identifier based on the current time in
 * microseconds.
 * @param string $prefix - Can be useful, for instance, if you generate
 * identifiers simultaneously on several hosts that might happen to generate
 * the identifier at the same microsecond.  With an empty prefix, the returned
 * string will be 13 characters long. If more_entropy is TRUE, it will be 23
 * characters.
 * @param bool $more_entropy - If set to TRUE, uniqid() will add additional
 * entropy (using the combined linear congruential generator) at the end of
 * the return value, which should make the results more unique.
 * @return string - Returns the unique identifier, as a string.
 */
<<__Native>>
function uniqid(string $prefix = "",
                bool $more_entropy = false): string;

/** Unpacks from a binary string into an array according to the given format.
 * unpack() works slightly different from Perl as the unpacked data is stored
 * in an associative array. To accomplish this you have to name the different
 * format codes and separate them by a slash /.
 * @param string $format - See pack() for an explanation of the format codes.
 * @param string $data - The packed data.
 * @return mixed - Returns an associative array containing unpacked elements
 * of binary string.
 */
<<__Native, __IsFoldable, __Rx>>
function unpack(string $format,
                string $data): mixed;

/** Returns three samples representing the average system load (the number of
 * processes in the system run queue) over the last 1, 5 and 15 minutes,
 * respectively.
 * @return array - Returns an array with three samples (last 1, 5 and 15
 * minutes).
 */
<<__Native>>
function sys_getloadavg(): varray;

/** Casts a given value to a string.
 * @param mixed $v - The value being casted to a string.
 * @return string - The result of the string cast.
 */
<<__Native, __IsFoldable, __Rx>>
function hphp_to_string(mixed $v): string;

function __hhas_adata(string $incorrect_hhas_adata) {
  throw new Exception(
    "__hhas_adata may only be called with a scalar string argument."
  );
}

}

namespace __SystemLib {

/** max2() returns the max of two operands (optimized FCallBuiltin for max).
 * @param mixed $arg1 - The first operand of max.
 * @param mixed $arg2 - The second operand of max.
 * @return mixed - The max of two operands.
 */
<<__Native, __HipHopSpecific, __IsFoldable, __Rx>>
function max2(mixed $arg1, mixed $arg2): mixed;

/** min2() returns the min of two operands (optimized FCallBuiltin for min).
 * @param mixed $arg1 - The first operand of min.
 * @param mixed $arg2 - The second operand of min.
 * @return mixed - The min of two operands.
 */
<<__Native, __HipHopSpecific, __IsFoldable, __Rx>>
function min2(mixed $arg1, mixed $arg2): mixed;

}
