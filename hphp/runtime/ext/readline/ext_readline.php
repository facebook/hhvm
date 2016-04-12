<?hh

/**
 * Adds a line to the history
 *
 * @param string $line - The line to be added in the history.
 *
 * @return bool -
 */
<<__Native>>
function readline_add_history(string $line): bool;

/**
 * Clears the history
 *
 * @return bool -
 */
<<__Native>>
function readline_clear_history(): bool;

/**
 * Registers a completion function
 * @param mixed $function - You must supply either a callable or the name of an
 * existing function which accepts a partial command line and returns an array
 * of possible matches.
 * @return bool -
 */
<<__Native>>
function readline_completion_function(mixed $function): bool;


/**
 * Gets/sets various internal readline variables
 * @param string $varname - A variable name.
 * @param string $newvalue - If provided, this will be the new value of setting
 * @return mixed -
 */
<<__Native>>
function readline_info(?string $varname = null,
                       ?string $newvalue = null): mixed;

/**
 * Reads the history
 *
 * @param string $filename - Path to the filename containing the command
 *   history.
 *
 * @return bool -
 */
<<__Native>>
function readline_read_history(?string $filename = null): bool;

/**
 * Writes the history
 *
 * @param string $filename - Path to the saved file.
 *
 * @return bool -
 */
<<__Native>>
function readline_write_history(?string $filename = null): bool;

/**
 * Reads a line
 *
 * @param string $prompt - You may specify a string with which to prompt
 *   the user.
 *
 * @return string - Returns a single string from the user. The line
 *   returned has the ending newline removed.
 */
<<__Native>>
function readline(?string $prompt = null): mixed;
