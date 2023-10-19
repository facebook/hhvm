<?hh
<<file:__EnableUnstableFeatures("readonly")>>

namespace {
/* Finds whether the given variable is a boolean.
 */
<<__IsFoldable, __Native>>
function is_bool(readonly mixed $var)[]: bool;

/* Finds whether the type of the given variable is integer.  To test if a
 * variable is a number or a numeric string (such as form input, which is
 * always a string), you must use is_numeric().
 */
<<__IsFoldable, __Native>>
function is_int(readonly mixed $var)[]: bool;

<<__IsFoldable, __Native>>
function is_integer(readonly mixed $var)[]: bool;

<<__IsFoldable, __Native>>
function is_long(readonly mixed $var)[]: bool;

/* Finds whether the type of the given variable is float.  To test if a
 * variable is a number or a numeric string (such as form input, which is
 * always a string), you must use is_numeric().
 */
<<__IsFoldable, __Native>>
function is_float(readonly mixed $var)[]: bool;

<<__IsFoldable, __Native>>
function is_double(readonly mixed $var)[]: bool;

<<__IsFoldable, __Native>>
function is_real(readonly mixed $var)[]: bool;

/* Finds whether the given variable is numeric. Numeric strings consist of
 * optional sign, any number of digits, optional decimal part and optional
 * exponential part. Thus +0123.45e6 is a valid numeric value. Hexadecimal
 * notation (0xFF) is allowed too but only without sign, decimal and
 * exponential part.
 */
<<__IsFoldable, __Native>>
function is_numeric(readonly mixed $var)[]: bool;

/* Finds whether the type given variable is string.
 */
<<__IsFoldable, __Native>>
function is_string(readonly mixed $var)[]: bool;

/* Finds whether the given variable is a scalar.  Scalar variables are those
 * containing an integer, float, string or boolean. Types array, object and
 * resource are not scalar.  is_scalar() does not consider resource type
 * values to be scalar as resources are abstract datatypes which are currently
 * based on integers. This implementation detail should not be relied upon, as
 * it may change.
 */
<<__IsFoldable, __Native>>
function is_scalar(readonly mixed $var)[]: bool;

/* Finds whether the given variable is an array.
 */
<<__IsFoldable, __Native>>
function is_array(readonly mixed $var): bool;

/* Finds whether the given variable is an object.
 */
<<__IsFoldable, __Native>>
function is_object(readonly mixed $var)[]: bool;

/* Finds whether the given variable is a resource.
 */
<<__IsFoldable, __Native>>
function is_resource(readonly mixed $var)[]: bool;

/* Finds whether the given variable is NULL.
 */
<<__IsFoldable, __Native>>
function is_null(readonly mixed $var)[]: bool;

/* Returns the type of the PHP variable var. Warning Never use gettype() to
 * test for a certain type, since the returned string may be subject to change
 * in a future version. In addition, it is slow too, as it involves string
 * comparison. Instead, use the is_* functions.
 */
<<__IsFoldable, __Native>>
function gettype(readonly mixed $v)[]: string;

/* This function gets the type of the given resource.
 */
<<__IsFoldable, __Native>>
function get_resource_type(resource $handle)[]: string;

<<__IsFoldable, __Native>>
function boolval(mixed $var)[]: bool;

/* Returns the integer value of var, using the specified base for the
 * conversion (the default is base 10). intval() should not be used on
 * objects, as doing so will emit an E_NOTICE level error and return 1.
 */
<<__IsFoldable, __Native>>
function intval(mixed $var,
                int $base = 10)[]: int;

/* Gets the float value of var.
 */
<<__IsFoldable, __Native>>
function floatval(mixed $var)[]: float;

<<__IsFoldable, __Native>>
function doubleval(mixed $var)[]: float;

<<__IsFoldable, __Native>>
function strval(mixed $var)[]: string;

/* print_r() displays information about a variable in a way that's readable by
 * humans.  print_r(), var_dump() and var_export() will also show protected
 * and private properties of objects with PHP 5. Static class members will not
 * be shown.  Remember that print_r() will move the array pointer to the end.
 * Use reset() to bring it back to beginning.
 */
<<__Native>>
function print_r(mixed $expression,
                 bool $ret = false): mixed;

/* Pure version of print_r(). print_r() has side effects because
 * 1. when the 2nd arg is not set to true, we would write to std output
 * 2. impure implementation of __debugInfo may cause coeffects violation
 */
<<__Native>>
function print_r_pure(mixed $expression)[]: mixed;

<<__Native>>
function var_export(mixed $expression,
                    bool $ret = false): mixed;

/* Pure version of var_export(). When var_export()'s 2nd arg
 * is not set to true, we would write to std output.
 */
<<__Native>>
function var_export_pure(mixed $expression)[]: mixed;

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
function var_dump(readonly mixed $arg1, mixed... $argv): void;

<<__Native>>
function debug_zval_dump(mixed $variable): void;

<<__Native>>
function debugger_dump(mixed $variable): void;

/**
 * Generates a storable representation of a value  This is useful for storing
 * or passing PHP values around without losing their type and structure.  To
 * make the serialized string into a PHP value again, use unserialize().
 *
 * Calls to serialize are foldable because only objects can invoke user-defined
 * code.
 */
<<__IsFoldable, __Native>>
function serialize(mixed $value): string;

/**
 * Pure variant of serialize.
 * Serializing objects with impure implementations of the `__sleep` method will
 * result in coeffect violations.
 */
<<__IsFoldable, __Native>>
function serialize_pure(mixed $value)[]: string;

<<__Native>>
function unserialize(
  string $str,
  darray<string, mixed> $options = darray[],
): mixed;

/**
 * Pure variant of unserialize.
 * Unserializing objects with impure implementations of the `__wakeup` method
 * will result in coeffect violations.
 */
<<__Native>>
function unserialize_pure(
  string $str,
  darray<string, mixed> $options = darray[],
)[]: mixed;

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
                   <<__OutOnly("darray")>>
                   inout mixed $arr): void;

}

namespace HH {

  /* Finds whether the given variable is a vec.
   */
  <<__Native, __IsFoldable>>
  function is_vec(readonly mixed $var)[]: bool;

  /* Finds whether the given variable is a dict.
   */
  <<__Native, __IsFoldable>>
  function is_dict(readonly mixed $var)[]: bool;

  /* Finds whether the given variable is a keyset.
   */
  <<__Native, __IsFoldable>>
  function is_keyset(readonly mixed $var)[]: bool;

  <<__Native, __IsFoldable>>
  function is_varray(readonly mixed $var): bool;

  <<__Native, __IsFoldable>>
  function is_darray(readonly mixed $var): bool;

  <<__Native, __IsFoldable>>
  function is_any_array(readonly mixed $var)[]: bool;

  <<__Native>>
  function is_php_array(readonly mixed $var)[]: bool;

  <<__Native, __IsFoldable>>
  function is_dict_or_darray(readonly mixed $var)[]: bool;

  <<__Native, __IsFoldable>>
  function is_vec_or_varray(readonly mixed $var)[]: bool;

  /*
   * Check if the input is an array-like containing only integer keys running
   * from 0 to N-1, in that order.
   */
  <<__Native, __IsFoldable>>
  function is_list_like(readonly AnyArray<arraykey, mixed> $var)[]: bool;

  <<__Native, __IsFoldable>>
  function is_meth_caller(readonly mixed $var)[]: bool;

 /*
  * Behaves like serialize() but takes an optional set of options.
  *
  * Options:
  *
  * disallowObjects - If true, throw an InvalidOperationException if serializing
  *                    an object. Note that this doesn't trigger on Collections
  * warnOnHackArrays - If true, emit a Hack array compat notice if serializing a
  *                    Hack array
  * warnOnPHPArrays  - If true, emit a Hack array compat notice if serializing a
  *                    PHP array
  * forcePHPArrays   - If true, serialize all Hack arrays as PHP arrays
  * keepDVArrays     - If true, use custom HHVM-specific serialization format,
  *                    which preserves varray and darray intact.
  * serializeProvenanceAndLegacy - If true, use unstable HHVM-specific format to
  *                    serialize provenance tag and legacy bit (hack array
  *                    migration primitives). The format, produced by this
  *                    option may change between HHVM releases in
  *                    non-backward-compatible manner, so it's not safe for
  *                    serialized data to leave HHVM process.
  */
  <<__Native, __IsFoldable>>
  function serialize_with_options(
    mixed $value,
    dict<string, mixed> $options = dict[],
  ): string;

  /*
   * This function returns an array of an object's properties in the same manner
   * as casting the object to an array.
   *
   * if ignore_late_init, just ignore unset late init members rather than
   * throwing an exception
   */
  <<__Native>>
  function object_prop_array(
    \object $obj,
    bool $ignore_late_init = false,
  )[]: darray<string, mixed>;

  /*
   * Return true if the <<__LateInit>> property (with name $prop) on the given
   * object is initialized to a value (and therefore will not throw when
   * accessed). Throws InvalidArgumentException if the property does not exist
   * or is inaccessible in the current context.
   */
  <<__Native, \NoDoc>>
  function is_late_init_prop_init(\object $obj, string $prop): bool;

  /*
   * Return true if the <<__LateInit>> static property (with name $prop) on the
   * class given by $cls is initialized to a value (and therefore will not throw
   * when accessed). Throws InvalidArgumentException if $cls is not a valid
   * classname, if the static property does not exist, or if the static property
   * is inaccessible in the current context.
   */
  <<__Native, \NoDoc>>
  function is_late_init_sprop_init(string $cls, string $prop): bool;

  /*
   * Does the key exist in the globals array shared between runtime
   * and code.
   */
  <<__Native>>
  function global_key_exists(string $key)[read_globals]: bool;
}

namespace HH\Lib\_Private\Native {
  /*
  * These functions all take Containers
  */
  <<__Native, __IsFoldable>>
  function first(
    mixed $iterable
  )[]: mixed;

  <<__Native, __IsFoldable>>
  function first_key(
    mixed $iterable
  )[]: mixed;

  <<__Native, __IsFoldable>>
  function last(
    mixed $iterable
  )[]: mixed;

  <<__Native, __IsFoldable>>
  function last_key(
    mixed $iterable
  )[]: mixed;
}
