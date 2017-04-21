<?php

trait A { public function foo() {return 1;} }
trait B { public function foo() {return 2;} }

class Thing {
  use A, B {
    A::foo insteadof A, B;
  }
}

$t = new Thing;
var_dump($t->foo());
