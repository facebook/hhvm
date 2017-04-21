<?php

trait A { public function foo() {return 1;} }
trait B { public function foo() {return 2;} }
trait C { }

class Thing {
  use A, B, C {
    A::foo insteadof B, C;
  }
}

$t = new Thing;
var_dump($t->foo());
