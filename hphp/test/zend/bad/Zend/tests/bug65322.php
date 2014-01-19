<?php

spl_autoload_register(function($class) {
    var_dump($class);
    class B {}
});

set_error_handler(function($_, $msg, $file) {
    var_dump($msg, $file);
    new B;
});

eval('class A { function a() {} function __construct() {} }');

?>