<?php

class A {
    const A = ['a' => ['b' => 'c']];
}

var_dump(A::A);
var_dump(A::A['a']);
var_dump(A::A['a']['b']);

?>
