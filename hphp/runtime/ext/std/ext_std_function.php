<?hh

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
function is_callable(mixed $callback, bool $syntax_only = false,
                     mixed &$callable_name = null): bool;

/**
 * Call a callback with an array of parameters
 *
 * @param callable $callback - The callable to be called.
 * @param array $param_arr - The parameters to be passed to the callback,
 *   as an indexed array.
 *
 * @return mixed - Returns the return value of the callback, or FALSE on
 *   error.
 */
<<__Native>>
function call_user_func_array(mixed $callback,
                              array $params): mixed;

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
 * Create an anonymous (lambda-style) function
 *
 * @param string $args - The function arguments.
 * @param string $code - The function code.
 *
 * @return string - Returns a unique function name as a string, or FALSE
 *   on error.
 */
<<__Native>>
function create_function(string $args,
                         string $code): string;

/**
 * Call a static method and pass the arguments as array
 *
 * @param callable $function - The function or method to be called. This
 *   parameter may be an , with the name of the class, and the method, or a
 *   , with a function name.
 * @param array $parameters -
 *
 * @return mixed - Returns the function result, or FALSE on error.
 */
<<__Native>>
function forward_static_call_array(mixed $function,
                                   array $parameters): mixed;

/**
 * Call a static method
 *
 * @param callable $function - The function or method to be called. This
 *   parameter may be an array, with the name of the class, and the method,
 *   or a string, with a function name.
 * @param mixed $parameters... - Zero or more parameters to be passed to the
 *   function.
 *
 * @return mixed - Returns the function result, or FALSE on error.
 */
<<__Native>>
function forward_static_call(mixed $function,
                             ...$parameters): mixed;

/**
 * Return an item from the argument list
 *
 * @param int $arg_num - The argument offset. Function arguments are
 *   counted starting from zero.
 *
 * @return mixed - Returns the specified argument, or FALSE on error.
 */
<<__Native>>
function func_get_arg(int $arg_num): mixed;

/**
 * Returns an array comprising a function's argument list
 *
 * @return mixed - Returns an array in which each element is a copy of
 *   the corresponding member of the current user-defined function's
 *   argument list. Returns false and raises warning when called
 *   from global scope.
 */
<<__Native>>
function func_get_args(): mixed;

/**
 * Returns the number of arguments passed to the function
 *
 * @return int - Returns the number of arguments passed into the current
 *   user-defined function.
 */
<<__Native>>
function func_num_args(): int;

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

namespace __SystemLib {

 /**
  * @param int $offset
  *
  * @return mixed
  */
  <<__Native, __HipHopSpecific>>
  function func_slice_args(int $offset): mixed;

 /**
  * @param int $arg_num
  *
  * @return mixed
  */
  <<__Native, __HipHopSpecific>>
  function func_get_arg_sl(int $arg_num): mixed;

 /**
  * @return mixed
  */
  <<__Native, __HipHopSpecific>>
  function func_get_args_sl(): mixed;

 /**
  * @return int
  */
  <<__Native, __HipHopSpecific>>
  function func_num_arg_(): int;

}
