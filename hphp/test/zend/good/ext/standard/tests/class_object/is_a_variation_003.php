<?hh
/* Prototype  : proto bool is_a(object object, string class_name)
 * Description: Returns true if the object is of this class or has this class as one of its parents
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class caseSensitivityTest {}
class caseSensitivityTestChild extends caseSensitivityTest {}
<<__EntryPoint>> function is_a_variation_003(): void {
echo "*** Testing is_a() : usage variations ***\n";

var_dump(is_a(new caseSensitivityTestChild, 'caseSensitivityTEST'));

echo "Done";
}
