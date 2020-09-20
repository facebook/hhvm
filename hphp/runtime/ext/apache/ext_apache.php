<?hh // partial

namespace {
/**
 * Get and set apache request notes
 *
 * @param string $note_name - The name of the note.
 * @param string $note_value - The value of the note.
 *
 * @return string - If called with one argument, it returns the current
 *   value of note note_name. If called with two arguments, it sets the
 *   value of note note_name to note_value and returns the previous value
 *   of note note_name. If the note cannot be retrieved, FALSE is returned.
 */
<<__Native>>
function apache_note(string $note_name,
                     ?string $note_value = ""): mixed;

/**
 * Set many apache request notes at once.
 * All keys must be strings.
 * All values must be strings.
 * Nullable values may not be used to remove keys as with apache_note().
 * This function returns nothing.
 */
<<__Native>>
function apache_notes(dict<string, string> $notes): void;

/**
 * Fetch all HTTP request headers
 *
 * @return array - An associative array of all the HTTP headers in the
 *   current request.
 */
<<__Native>>
function apache_request_headers(): darray<string, string>;

/**
 * Fetch all HTTP response headers
 *
 * @return array - An array of all Apache response headers on success.
 */
<<__Native>>
function apache_response_headers(): darray<string, string>;

/**
 * Set an Apache subprocess_env variable
 *
 * @param string $variable - The environment variable that's being set.
 * @param string $value - The new variable value.
 * @param bool $walk_to_top - Whether to set the top-level variable
 *   available to all Apache layers.
 *
 * @return bool -
 */
<<__Native>>
function apache_setenv(string $variable,
                       string $value,
                       bool $walk_to_top = false): bool;

/**
 * Fetch all HTTP request headers
 *
 * @return array - An associative array of all the HTTP headers in the
 *   current request.
 */
<<__Native>>
function getallheaders(): darray<string, string>;

<<__Native, __HipHopSpecific>>
function apache_get_config(): darray<string, mixed>;

}

namespace HH {
/**
 * Fetch all HTTP request headers, including duplicates
 *
 * @return array - An associative array of all the HTTP headers in the
 *   current request. The values in the array will be strings for uniquely
 *   specified headers, but arrays where a header was specified more than once.
 */
<<__Native, __HipHopSpecific>>
function get_headers_secure(): darray<string, varray<string>>;

}
