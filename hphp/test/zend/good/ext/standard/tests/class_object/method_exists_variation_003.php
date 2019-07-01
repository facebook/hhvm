<?hh
/* Prototype  : proto bool method_exists(object object, string method)
 * Description: Checks if the class method exists
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class caseSensitivityTest {
    public function myMethod() {}
}
<<__EntryPoint>> function main(): void {
echo "*** Testing method_exists() : variation ***\n";

var_dump(method_exists(new casesensitivitytest, 'myMetHOD'));
var_dump(method_exists('casesensiTivitytest', 'myMetHOD'));

echo "Done";
}
