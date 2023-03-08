<?hh
/* Prototype  : proto array get_declared_classes()
 * Description: Returns an array of all declared classes.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

<<__EntryPoint>> function get_declared_classes_variation1(): void {
echo "*** Testing get_declared_classes() : testing autoloaded classes ***\n";

echo "\n-- before instance is declared --\n";
var_dump(in_array('AutoLoaded', get_declared_classes()));

echo "\n-- after instance is declared --\n";
$class = new AutoLoaded();
var_dump(in_array('AutoLoaded', get_declared_classes()));

echo "\nDONE\n";
}
