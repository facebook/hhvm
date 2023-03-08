<?hh

<<__EntryPoint>>
function entrypoint_get_declared_interfaces_variation1(): void {
  /* Prototype  : proto array get_declared_interfaces()
   * Description: Returns an array of all declared interfaces.
   * Source code: Zend/zend_builtin_functions.c
   * Alias to functions:
   */


  echo "*** Testing get_declared_interfaces() : autoloading of interfaces ***\n";

  echo "\n-- before interface is used --\n";
  var_dump(in_array('AutoInterface', get_declared_interfaces()));

  require_once(__DIR__.'/get_declared_interfaces_variation1.inc');

  echo "\n-- after interface is used --\n";
  var_dump(in_array('AutoInterface', get_declared_interfaces()));

  echo "\nDONE\n";
}
