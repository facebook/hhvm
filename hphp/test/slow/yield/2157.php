<?php

class A {
  public function foo() {
    $this->bar(function() {
yield 1;
 yield 2;
 yield 3;
}
);
  }
  public function bar(Closure $c) {
    $a = $c();
    foreach ($a as $b) {
      echo $b."\n";
    }
  }
}
$a = new A();
$a->foo();
