<?hh // partial

namespace {

/**
 * Verify that the contents of a variable can be called as a function
 *
 * @param callable $name - The callback function to check
 * @param bool $syntax_only - If set to TRUE the function only verifies that
 *   name might be a function or method. It will only reject simple variables
 *   that are not strings, or an array that does not have a valid structure to
 *   be used as a callback. The valid ones are supposed to have only 2 entries,
 *   the first of which is an object or a string, and the second a string.
 * @param string $callable_name - Receives the "callable name" by reference.
 *
 * @return mixed -  Returns TRUE if name is callable, FALSE otherwise.
 */
<<__Native, __Rx>>
function is_callable(mixed $callback, bool $syntax_only = false,
                     mixed &$callable_name = null): bool;

/**
 * Call a callback with an array of parameters
 *
 * @param callable $callback - The callable to be called.
 * @param mixed $param_arr - The parameters to be passed to the callback,
 *   as an indexed array or collection.
 *
 * @return mixed - Returns the return value of the callback, or FALSE on
 *   error.
 */
<<__Native>>
function call_user_func_array(mixed $callback,
                              mixed $params): mixed;

/**
 * Call the callback given by the first parameter
 *
 * @param callable $callback - The callable to be called.
 * @param mixed $parameters... - Zero or more parameters to be passed to the
 *   callback.    Note that the parameters for call_user_func() are not
 *   passed by reference.  call_user_func() example and references
 *
 * @return mixed - Returns the return value of the callback, or FALSE on
 *   error.
 */
<<__Native>>
function call_user_func(mixed $callback,
                        ...$parameters): mixed;

/**
 * Returns the number of arguments passed to the function
 *
 * @return int - Returns the number of arguments passed into the current
 *   user-defined function.
 */
function func_num_args(): int {
  throw new InvalidArgumentException(
    "Unsupported dynamic call of func_num_args()",
  );
}

/**
 * Return TRUE if the given function has been defined
 *
 * @param string $function_name - The function name, as a string.
 * @param bool $autoload - Whether to try to autoload.
 *
 * @return bool - Returns TRUE if function_name exists and is a function,
 *   FALSE otherwise.    This function will return FALSE for constructs,
 *   such as include_once() and echo().
 */
<<__Native, __Rx>>
function function_exists(string $function_name, bool $autoload = true): bool;

/**
 * Returns an array of all defined functions
 *
 * @return array - Returns a multidimensional array containing a list of
 *   all defined functions, both built-in (internal) and user-defined. The
 *   internal functions will be accessible via $arr["internal"], and the
 *   user defined ones using $arr["user"] (see example below).
 */
<<__Native>>
function get_defined_functions(): array;

/**
 * Register a function for execution on shutdown
 *
 * @param callable $callback - The shutdown callback to register.   The
 *   shutdown callbacks are executed as the part of the request, so it's
 *   possible to send output from them and access output buffers.
 * @param mixed $parameters... - It is possible to pass parameters to the
 *   shutdown function by passing additional parameters.
 *
 * @return void
 */
<<__Native>>
function register_shutdown_function(mixed $callback,
                                    ...$parameters): void;

/**
 * @param callable $callback
 * @param mixed $parameters...
 *
 * @return void
 */
<<__Native, __HipHopSpecific>>
function register_postsend_function(mixed $callback,
                                    ...$parameters): void;

}
