<?hh
/* Prototype  : array get_loaded_extensions  ([ bool $zend_extensions= false  ] )
 * Description:  Returns an array with the names of all modules compiled and loaded
 * Source code: Zend/zend_builtin_functions.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing get_loaded_extensions() : basic functionality ***\n";

echo "Get loaded extensions\n";
var_dump(get_loaded_extensions());

echo "===DONE===\n";
}
