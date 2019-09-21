<?hh // partial

namespace {
/* Finds whether the given variable is a boolean.
 */
<<__IsFoldable, __Native, __Rx>>
function is_bool(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the type of the given variable is integer.  To test if a
 * variable is a number or a numeric string (such as form input, which is
 * always a string), you must use is_numeric().
 */
<<__IsFoldable, __Native, __Rx>>
function is_int(<<__MaybeMutable>> mixed $var): bool;

<<__IsFoldable, __Native, __Rx>>
function is_integer(<<__MaybeMutable>> mixed $var): bool;

<<__IsFoldable, __Native, __Rx>>
function is_long(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the type of the given variable is float.  To test if a
 * variable is a number or a numeric string (such as form input, which is
 * always a string), you must use is_numeric().
 */
<<__IsFoldable, __Native, __Rx>>
function is_float(<<__MaybeMutable>> mixed $var): bool;

<<__IsFoldable, __Native, __Rx>>
function is_double(<<__MaybeMutable>> mixed $var): bool;

<<__IsFoldable, __Native, __Rx>>
function is_real(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the given variable is numeric. Numeric strings consist of
 * optional sign, any number of digits, optional decimal part and optional
 * exponential part. Thus +0123.45e6 is a valid numeric value. Hexadecimal
 * notation (0xFF) is allowed too but only without sign, decimal and
 * exponential part.
 */
<<__IsFoldable, __Native, __Rx>>
function is_numeric(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the type given variable is string.
 */
<<__IsFoldable, __Native, __Rx>>
function is_string(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the given variable is a scalar.  Scalar variables are those
 * containing an integer, float, string or boolean. Types array, object and
 * resource are not scalar.  is_scalar() does not consider resource type
 * values to be scalar as resources are abstract datatypes which are currently
 * based on integers. This implementation detail should not be relied upon, as
 * it may change.
 */
<<__IsFoldable, __Native, __Rx>>
function is_scalar(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the given variable is an array.
 */
<<__IsFoldable, __Native, __Rx>>
function is_array(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the given variable is an object.
 */
<<__IsFoldable, __Native, __Rx>>
function is_object(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the given variable is a resource.
 */
<<__IsFoldable, __Native, __Rx>>
function is_resource(<<__MaybeMutable>> mixed $var): bool;

/* Finds whether the given variable is NULL.
 */
<<__IsFoldable, __Native, __Rx>>
function is_null(<<__MaybeMutable>> mixed $var): bool;

/* Returns the type of the PHP variable var. Warning Never use gettype() to
 * test for a certain type, since the returned string may be subject to change
 * in a future version. In addition, it is slow too, as it involves string
 * comparison. Instead, use the is_* functions.
 */
<<__IsFoldable, __Native, __Rx>>
function gettype(<<__MaybeMutable>> mixed $v): string;

/* This function gets the type of the given resource.
 */
<<__IsFoldable, __Native, __Rx>>
function get_resource_type(<<__MaybeMutable>> resource $handle): string;

<<__IsFoldable, __Native, __Rx>>
function boolval(mixed $var): bool;

/* Returns the integer value of var, using the specified base for the
 * conversion (the default is base 10). intval() should not be used on
 * objects, as doing so will emit an E_NOTICE level error and return 1.
 */
<<__IsFoldable, __Native, __Rx>>
function intval(mixed $var,
                int $base = 10): int;

/* Gets the float value of var.
 */
<<__IsFoldable, __Native, __Rx>>
function floatval(mixed $var): float;

<<__IsFoldable, __Native, __Rx>>
function doubleval(mixed $var): float;

<<__IsFoldable, __Native, __Rx>>
function strval(mixed $var): string;

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
 *
 * Calls to serialize are foldable because only objects can invoke user-defined
 * code.
 */
<<__IsFoldable, __Native, __Rx>>
function serialize(mixed $value): string;

<<__Native, __Rx>>
function unserialize(string $str,
                     darray $options = darray[]): mixed;

/* Imports GET/POST/Cookie variables into the global scope. It is useful if
 * you disabled register_globals, but would like to see some variables in the
 * global scope.
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

/*
 * Parses str as if it were the query string passed via a URL and sets $arr.
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
function parse_str(string $str,
                   <<__OutOnly("KindOfArray")>>
                   inout mixed $arr): void;

}

namespace HH {

  /* Finds whether the given variable is a vec.
   */
  <<__Native, __IsFoldable, __Rx>>
  function is_vec(<<__MaybeMutable>> mixed $var): bool;

  /* Finds whether the given variable is a dict.
   */
  <<__Native, __IsFoldable, __Rx>>
  function is_dict(<<__MaybeMutable>> mixed $var): bool;

  /* Finds whether the given variable is a keyset.
   */
  <<__Native, __IsFoldable, __Rx>>
  function is_keyset(<<__MaybeMutable>> mixed $var): bool;

  <<__Native, __IsFoldable, __Rx>>
  function is_varray(<<__MaybeMutable>> mixed $var): bool;

  <<__Native, __IsFoldable, __Rx>>
  function is_darray(<<__MaybeMutable>> mixed $var): bool;

  <<__Native, __IsFoldable, __Rx>>
  function is_any_array(<<__MaybeMutable>> mixed $var): bool;

  /*
   * Check if the input is an array-like containing only integer keys running
   * from 0 to N-1, in that order.
   */
  <<__Native, __IsFoldable, __Rx>>
  function is_list_like(<<__MaybeMutable>> arraylike $var): bool;

  <<__Native, __IsFoldable, __Rx>>
  function is_meth_caller(<<__MaybeMutable>> mixed $var): bool;

 /*
  * Behaves like serialize() but takes an optional set of options.
  *
  * Options:
  *
  * warnOnHackArrays - If true, emit a Hack array compat notice if serializing a
  *                    Hack array
  * warnOnPHPArrays  - If true, emit a Hack array compat notice if serializing a
  *                    PHP array
  * forcePHPArrays   - If true, serialize all Hack arrays as PHP arrays
  */
  <<__Native, __IsFoldable>>
  function serialize_with_options(mixed $value, dict $options = dict[]): string;

  /*
   * This function returns an array of an object's properties in the same manner
   * as casting the object to an array.
   */
  <<__Native>>
  function object_prop_array(object $obj): darray;

  /*
   * Return true if the <<__LateInit>> property (with name $prop) on the given
   * object is initialized to a value (and therefore will not throw when
   * accessed). Throws InvalidArgumentException if the property does not exist
   * or is inaccessible in the current context.
   */
  <<__Native>>
  function is_late_init_prop_init(object $obj, string $prop): bool;

  /*
   * Return true if the <<__LateInit>> static property (with name $prop) on the
   * class given by $cls is initialized to a value (and therefore will not throw
   * when accessed). Throws InvalidArgumentException if $cls is not a valid
   * classname, if the static property does not exist, or if the static property
   * is inaccessible in the current context.
   */
  <<__Native>>
  function is_late_init_sprop_init(string $cls, string $prop): bool;

  /*
   * Return all of the keys of the globals array shared between
   * runtime and user code. Currently backed by $GLOBALS.
   */
  <<__Native>>
  function global_keys(): keyset<string>;

  /*
   * Does the key exist in the globals array shared between runtime
   * and code.
   */
  <<__Native>>
  function global_key_exists(string $key): bool;
}

namespace HH\Lib\_Private\Native {
  /*
  * container intrinsic for HH\traversable
  */
  <<__Native, __IsFoldable, __Rx, __AtMostRxAsArgs>>
  function first(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>>
    mixed $iterable
  ): mixed;

  <<__Native, __IsFoldable, __Rx, __AtMostRxAsArgs>>
  function first_key(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>>
    mixed $iterable
  ): mixed;

  <<__Native, __IsFoldable, __Rx, __AtMostRxAsArgs>>
  function last(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>>
    mixed $iterable
  ): mixed;

  <<__Native, __IsFoldable, __Rx, __AtMostRxAsArgs>>
  function last_key(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>>
    mixed $iterable
  ): mixed;
}
