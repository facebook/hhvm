<?php

class A {
    const C = 'A::C';

    public function f() {
        return function() {
            return self::C;
        };
    }
}

class B {
    const C = 'B::C';
}

$f = (new A)->f();
var_dump($f->bindTo(null, 'B')());

?>
