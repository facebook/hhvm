<?php
spl_autoload_register(function ($class) {
    var_dump("Loading $class");
    eval('class Bar {}');
});

class Foo
{
    function __sleep()
    {
        new Bar;
        return array();
    }
}

session_start();
$_SESSION['foo'] = new Foo;

?>