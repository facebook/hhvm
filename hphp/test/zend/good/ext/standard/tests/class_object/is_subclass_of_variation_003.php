<?hh
/* Prototype  : proto bool is_subclass_of(object object, string class_name)
 * Description: Returns true if the object has this class as one of its parents
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class caseSensitivityTest {}
class caseSensitivityTestChild extends caseSensitivityTest {}
<<__EntryPoint>> function is_subclass_of_variation_003(): void {
echo "*** Testing is_subclass_of() : usage variations ***\n";

echo "*** Testing is_a() : usage variations ***\n";

var_dump(is_subclass_of('caseSensitivityTestChild', 'caseSensitivityTest'));

echo "Done";
}
