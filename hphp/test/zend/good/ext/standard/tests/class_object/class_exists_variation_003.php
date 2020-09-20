<?hh
/* Prototype  : proto bool class_exists(string classname [, bool autoload])
 * Description: Checks if the class exists
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class caseSensitivityTest {}
<<__EntryPoint>> function main(): void {
var_dump(class_exists('casesensitivitytest'));
echo "Done";
}
