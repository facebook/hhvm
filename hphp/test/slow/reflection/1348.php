<?php

interface A {
}
interface B extends A {
}
interface C extends B {
}
class D implements A {
}
$d = new D;
var_dump(is_a($d, 'A'));
var_dump(is_a($d, 'B'));
var_dump(is_a('B', 'A', true));
var_dump(is_a('B', 'B', true));
var_dump(is_a('C', 'A', true));
