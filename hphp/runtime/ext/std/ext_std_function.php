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
<<__Native>>
function is_callable(mixed $callback, bool $syntax_only = false)[]: bool;

<<__Native>>
function is_callable_with_name(mixed $callback,
                               bool $syntax_only,
                               <<__OutOnly>>
                               inout mixed $callable_name)[]: bool;

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
<<__Native("NoRecording")>>
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
<<__Native("NoRecording")>>
function call_user_func(mixed $callback,
                        mixed... $parameters): mixed;

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
<<__Native>>
function function_exists(string $function_name, bool $autoload = true)[]: bool;

/**
 * Returns an array of all defined functions
 *
 * @return darray - Returns a multidimensional array containing a list of
 *   all defined functions, both built-in (internal) and user-defined. The
 *   internal functions will be accessible via $arr["internal"], and the
 *   user defined ones using $arr["user"] (see example below).
 */
<<__Native>>
function get_defined_functions(): shape(
  'internal' => vec<string>,
  'user' => vec<string>,
);

/**
 * Register a function for execution on shutdown
 *
 * @param callable $callback - The shutdown callback to register.   The
 *   shutdown callbacks are executed as the part of the request, so it's
 *   possible to send output from them and access output buffers.
 *
 * @return void
 */
<<__Native("NoRecording")>>
function register_shutdown_function(mixed $callback): void;

/**
 * @param callable $callback
 *
 * @return void
 */
<<__Native("NoRecording")>>
function register_postsend_function(mixed $callback): void;

}

namespace HH {
  /**
   * Get function name from fun
   * @param mixed $fun
   * @return function name
   */
  <<__Native>>
  function fun_get_function(mixed $fun)[]: string;
}
