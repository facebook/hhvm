<?php
class PHPUnit_Util_ErrorHandler
{
    public static function handleError($errno, $errstr, $errfile, $errline)
    {
        throw new RuntimeException;
    }
}

class A {
    public function getX() {
        return NULL;
    }
}

class B {
    public function foo() {
        $obj    = new A;
        $source = &$obj->getX();
    }

    public function bar() {
        $m = new ReflectionMethod('B', 'foo');
        $m->invoke($this);
    }
}

set_error_handler(
  array('PHPUnit_Util_ErrorHandler', 'handleError'), E_ALL | E_STRICT
);
            
$o = new B;
$o->bar();
?>