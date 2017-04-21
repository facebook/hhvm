<?php

trait A { public function bar() { return 1; } }

class Thing {
  use A { A::foo insteadof A; }
}

$t = new Thing;
var_dump($t->foo());
