<?hh
/* Prototype  : proto array get_class_methods(mixed class)
 * Description: Returns an array of method names for class or class instance.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

/*
 * Test basic behaviour with existing class and non-existent class.
 */

class C {
    function f() {}
    function g() {}
    function h() {}
}

class D {}
<<__EntryPoint>> function main(): void {
echo "*** Testing get_class_methods() : basic functionality ***\n";

echo "Argument is class name:\n";
var_dump( get_class_methods("C") );
echo "Argument is class instance:\n";
$c = new C;
var_dump( get_class_methods($c) );

echo "Argument is name of class which has no methods:\n";
var_dump( get_class_methods("D") );

echo "Argument is non existent class:\n";
var_dump( get_class_methods("NonExistent") );

echo "Done";
}
