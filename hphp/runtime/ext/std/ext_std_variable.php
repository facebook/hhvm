<?hh

namespace {
/* Finds whether the given variable is a boolean.
 */
<<__Native, __ParamCoerceModeFalse>>
function is_bool(mixed $var): bool;

/* Finds whether the type of the given variable is integer.  To test if a
 * variable is a number or a numeric string (such as form input, which is
 * always a string), you must use is_numeric().
 */
<<__Native, __ParamCoerceModeFalse>>
function is_int(mixed $var): bool;

<<__Native, __ParamCoerceModeFalse>>
function is_integer(mixed $var): bool;

<<__Native, __ParamCoerceModeFalse>>
function is_long(mixed $var): bool;

/* Finds whether the type of the given variable is float.  To test if a
 * variable is a number or a numeric string (such as form input, which is
 * always a string), you must use is_numeric().
 */
<<__Native, __ParamCoerceModeFalse>>
function is_float(mixed $var): bool;

<<__Native, __ParamCoerceModeFalse>>
function is_double(mixed $var): bool;

<<__Native, __ParamCoerceModeFalse>>
function is_real(mixed $var): bool;

/* Finds whether the given variable is numeric. Numeric strings consist of
 * optional sign, any number of digits, optional decimal part and optional
 * exponential part. Thus +0123.45e6 is a valid numeric value. Hexadecimal
 * notation (0xFF) is allowed too but only without sign, decimal and
 * exponential part.
 */
<<__Native>>
function is_numeric(mixed $var): bool;

/* Finds whether the type given variable is string.
 */
<<__Native, __ParamCoerceModeFalse>>
function is_string(mixed $var): bool;

/* Finds whether the given variable is a scalar.  Scalar variables are those
 * containing an integer, float, string or boolean. Types array, object and
 * resource are not scalar.  is_scalar() does not consider resource type
 * values to be scalar as resources are abstract datatypes which are currently
 * based on integers. This implementation detail should not be relied upon, as
 * it may change.
 */
<<__Native>>
function is_scalar(mixed $var): bool;

/* Finds whether the given variable is an array.
 */
<<__Native, __ParamCoerceModeFalse>>
function is_array(mixed $var): bool;

/* Finds whether the given variable is an object.
 */
<<__Native, __ParamCoerceModeFalse>>
function is_object(mixed $var): bool;

/* Finds whether the given variable is a resource.
 */
<<__Native, __ParamCoerceModeFalse>>
function is_resource(mixed $var): bool;

/* Finds whether the given variable is NULL.
 */
<<__Native, __ParamCoerceModeFalse>>
function is_null(mixed $var): bool;

/* Returns the type of the PHP variable var. Warning Never use gettype() to
 * test for a certain type, since the returned string may be subject to change
 * in a future version. In addition, it is slow too, as it involves string
 * comparison. Instead, use the is_* functions.
 */
<<__Native>>
function gettype(mixed $v): string;

/* This function gets the type of the given resource.
 */
<<__Native>>
function get_resource_type(resource $handle): string;

<<__Native>>
function boolval(mixed $var): bool;

/* Returns the integer value of var, using the specified base for the
 * conversion (the default is base 10). intval() should not be used on
 * objects, as doing so will emit an E_NOTICE level error and return 1.
 */
<<__Native>>
function intval(mixed $var,
                int $base = 10): int;

/* Gets the float value of var.
 */
<<__Native>>
function floatval(mixed $var): float;

<<__Native>>
function doubleval(mixed $var): float;

<<__Native>>
function strval(mixed $var): string;

/* Set the type of variable var to type.
 */
<<__Native>>
function settype(mixed &$var,
                 string $type): bool;

/* print_r() displays information about a variable in a way that's readable by
 * humans.  print_r(), var_dump() and var_export() will also show protected
 * and private properties of objects with PHP 5. Static class members will not
 * be shown.  Remember that print_r() will move the array pointer to the end.
 * Use reset() to bring it back to beginning.
 */
<<__Native>>
function print_r(mixed $expression,
                 bool $ret = false): mixed;

<<__Native>>
function var_export(mixed $expression,
                    bool $ret = false): mixed;

/* Dumps information about a variable
 *
 * This function displays structured information about one or more expressions
 * that includes its type and value. Arrays and objects are explored
 * recursively with values indented to show structure.
 *
 * @param mixed $var - Variable to dump
 */
/* Dumps a string representation of an internal zend value to output.
 */
<<__Native("NoFCallBuiltin")>>
function var_dump(mixed $arg1, ...$argv): void;

<<__Native>>
function debug_zval_dump(mixed $variable): void;

/* Generates a storable representation of a value  This is useful for storing
 * or passing PHP values around without losing their type and structure.  To
 * make the serialized string into a PHP value again, use unserialize().
 */
<<__Native>>
function serialize(mixed $value): string;

<<__Native>>
function unserialize(string $str,
                     array $class_whitelist = []): mixed;

/* This function returns a multidimensional array containing a list of all
 * defined variables, be they environment, server or user-defined
 * variables, within the scope in which get_defined_vars() is called.
 */
<<__Native>>
function get_defined_vars(): array;

/* Imports GET/POST/Cookie variables into the global scope. It is useful if
 * you disabled register_globals, but would like to see some variables in the
 * global scope.  If you're interested in importing other variables into the
 * global scope, such as $_SERVER, consider using extract().
 */
function import_request_variables(string $types,
                                  string $prefix = ""): bool {
  throw new Exception("It is bad coding practice to remove scoping of ".
                      "variables just to achieve coding convenience, ".
                      "esp. in a language that encourages global ".
                      "variables. This is possible to implement ".
                      "though, by declaring those global variables ".
                      "beforehand and assign with scoped ones when ".
                      "this function is called.");
}

/* Import variables from an array into the current symbol table.  Checks each
 * key to see whether it has a valid variable name. It also checks for
 * collisions with existing variables in the symbol table.
 */
<<__Native>>
function extract(mixed &$var_array,
                 int $extract_type = EXTR_OVERWRITE,
                 string $prefix = ""): int;

/*
 * Parses str as if it were the query string passed via a URL and sets
 * variables in the current scope.
 *
 * To get the current QUERY_STRING, you may use the variable
 * $_SERVER['QUERY_STRING']. Also, you may want to read the section on
 * variables from external sources.
 *
 * The magic_quotes_gpc setting affects the output of this function, as
 * parse_str() uses the same mechanism that PHP uses to populate the $_GET,
 * $_POST, etc. variables.
 */
<<__Native>>
function parse_str(string $str, mixed &$arr = null): void;

}

/*
 * Several of the above functions can affect the variable environment of the
 * their caller.  In order to allow the JIT to make more aggressive
 * optimizations, we have an option that disables dynamic calls to these
 * functions---to make that work, non-dynamic calls are rewritten to call these
 * __SystemLib versions, which are still allowed to modify the caller variable
 * environment when the option is enabled.
 */
namespace __SystemLib {
  <<__Native>>
  function extract(mixed &$var_array,
                   int $extract_type = EXTR_OVERWRITE,
                   string $prefix = ""): int;

  <<__Native>>
  function get_defined_vars(): array;

  <<__Native>>
  function parse_str(string $str, mixed &$arr = null): void;
}
