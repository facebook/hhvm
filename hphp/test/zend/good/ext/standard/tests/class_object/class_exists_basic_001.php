<?hh
/* Prototype  : proto bool class_exists(string classname [, bool autoload])
 * Description: Checks if the class exists
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing class_exists() : basic functionality ***\n";

echo "Calling class_exists() on non-existent class with autoload explicitly enabled:\n";
var_dump( class_exists('C', true) );
echo "\nCalling class_exists() on existing class with autoload explicitly enabled:\n";
var_dump( class_exists('stdClass', true) );

echo "\nCalling class_exists() on non-existent class with autoload explicitly enabled:\n";
var_dump( class_exists('D', false) );
echo "\nCalling class_exists() on existing class with autoload explicitly disabled:\n";
var_dump( class_exists('stdClass', false) );

echo "\nCalling class_exists() on non-existent class with autoload unspecified:\n";
var_dump( class_exists('E') );
echo "\nCalling class_exists() on existing class with autoload unspecified:\n";
var_dump( class_exists('stdClass') );

echo "Done";
}
