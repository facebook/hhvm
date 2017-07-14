<?php

interface A {
    function method(int $p);
}

class B implements A {
    function method(?int $p) { }
}

$b = new B();
$b->method(null);

