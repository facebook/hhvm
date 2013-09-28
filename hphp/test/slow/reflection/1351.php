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
var_dump(is_subclass_of($d, 'A'));
var_dump(is_subclass_of($d, 'B'));
var_dump(is_subclass_of('B', 'A'));
var_dump(is_subclass_of('B', 'B'));
var_dump(is_subclass_of('C', 'A'));
