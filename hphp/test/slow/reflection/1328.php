<?php

class A {
 public function foo() {
}
}
$x = new ReflectionMethod('A::foo');
var_dump($x->name, $x->class);
