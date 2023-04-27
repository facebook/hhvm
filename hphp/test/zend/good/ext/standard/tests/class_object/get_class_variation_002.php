<?hh
/* Prototype  : proto string get_class([object object])
 * Description: Retrieves the class name
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class caseSensitivityTest {}
<<__EntryPoint>> function get_class_variation_002(): void {
var_dump(get_class(new casesensitivitytest));
echo "Done";
}
