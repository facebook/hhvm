<?hh

/*
 * proto bool function_exists(string function_name)
 * Function is implemented in Zend/zend_builtin_functions.c
 */
<<__DynamicallyCallable>>
function f() :mixed{}
class C {
  <<__DynamicallyCallable>>
  static function f() :mixed{}
}
<<__EntryPoint>>
function main_entry(): void {
  echo "*** Testing function_exists() : basic functionality ***\n";

  echo "Internal function: ";
  var_dump(function_exists('function_exists'));

  echo "User defined function: ";
  var_dump(function_exists('f'));

  echo "Case sensitivity: ";
  var_dump(function_exists('F'));

  echo "Non existent function: ";
  var_dump(function_exists('g'));

  echo "Method: ";
  var_dump(function_exists('C::f'));
  echo "===Done===";
}
