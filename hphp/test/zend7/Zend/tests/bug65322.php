<?php

spl_autoload_register(function($class) {
    var_dump($class);
    class X {}
});

set_error_handler(function($_, $msg, $file) {
    var_dump($msg, $file);
    new X;
});

/* This is just a particular example of a non-fatal compile-time error
 * If this breaks in future, just find another example and use it instead */
eval('class A { function test() { } } class B extends A { function test($a) { } }');

?>
