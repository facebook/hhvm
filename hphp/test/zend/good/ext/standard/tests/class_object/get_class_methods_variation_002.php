<?hh
/* Prototype  : proto array get_class_methods(mixed class)
 * Description: Returns an array of method names for class or class instance.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class caseSensitivityTest {
    function MyMeThOd() {}
}
<<__EntryPoint>> function main(): void {
echo "*** Testing get_class_methods() : usage variations ***\n";

var_dump( get_class_methods('CasesensitivitytesT') );

echo "Done";
}
