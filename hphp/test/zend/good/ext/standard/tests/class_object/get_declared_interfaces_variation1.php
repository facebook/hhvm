<?php
/* Prototype  : proto array get_declared_interfaces()
 * Description: Returns an array of all declared interfaces. 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */


echo "*** Testing get_declared_interfaces() : autoloading of interfaces ***\n";

function __autoload($class_name) {
    require_once $class_name . '.inc';
}

echo "\n-- before interface is used --\n";
var_dump(in_array('AutoInterface', get_declared_interfaces()));


echo "\n-- after interface is used --\n";
class Implementor implements AutoInterface {}
var_dump(in_array('AutoInterface', get_declared_interfaces()));

echo "\nDONE\n";
?>
