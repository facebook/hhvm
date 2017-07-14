<?php

interface A {
    function method(?int $i): ?int;
}

class B implements A {
    function method(?int $i): ?int {
        return $i;
    }
}

$b = new B();
var_dump($b->method(null));
var_dump($b->method(1));

