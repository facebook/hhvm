<?php

class A {
}
 class B extends A {
}
$a = new A;
 $b = new B;
var_dump(is_a($a, 'A'));
var_dump(is_a($a, 'B'));
var_dump(is_a($b, 'A'));
var_dump(is_a($b, 'A', true));
