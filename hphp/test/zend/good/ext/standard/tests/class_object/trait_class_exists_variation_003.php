<?hh
/* Prototype  : proto bool trait_exists(string traitname [, bool autoload])
 * Description: Checks if the trait exists
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

trait caseSensitivityTest {}
<<__EntryPoint>> function trait_class_exists_variation_003(): void {
var_dump(trait_exists('casesensitivitytest'));
echo "Done";
}
