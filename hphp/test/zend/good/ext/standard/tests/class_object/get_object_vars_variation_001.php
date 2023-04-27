<?hh
/* Prototype  : proto array get_object_vars(object obj)
 * Description: Returns an array of object properties
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class A {
    public static $var = 'hello';
}
<<__EntryPoint>> function get_object_vars_variation_001(): void {
$a = new A;
var_dump(get_object_vars($a));
}
