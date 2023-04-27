<?hh
/* Prototype  : proto string get_parent_class([mixed object])
 * Description: Retrieves the parent class name for object or class or current scope.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

//  Note: basic use cases in Zend/tests/010.phpt

class caseSensitivityTest {}
class caseSensitivityTestChild extends caseSensitivityTest {}
<<__EntryPoint>> function get_parent_class_variation_001(): void {
echo "*** Testing get_parent_class() : variation ***\n";

var_dump(get_parent_class('CasesensitivitytestCHILD'));
var_dump(get_parent_class(new CasesensitivitytestCHILD));

echo "Done";
}
