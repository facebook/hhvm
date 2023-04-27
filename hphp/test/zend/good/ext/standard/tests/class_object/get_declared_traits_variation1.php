<?hh

<<__EntryPoint>>
function entrypoint_get_declared_traits_variation1(): void {
  /* Prototype  : proto array get_declared_traits()
   * Description: Returns an array of all declared traits.
   * Source code: Zend/zend_builtin_functions.c
   * Alias to functions:
   */


  echo "*** Testing get_declared_traits() : testing autoloaded traits ***\n";

  echo "\n-- before instance is declared --\n";
  var_dump(in_array('AutoTrait', get_declared_traits()));

  require_once(__DIR__.'/get_declared_traits_variation1.inc');

  echo "\n-- after use is declared --\n";

  var_dump(in_array('AutoTrait', get_declared_traits()));

  echo "\nDONE\n";
}
