<?php

class Base {
  private function fun() { return "fun"; }
  function __call($x, $y) {
    return "__call";
  }
}

class Foo extends Base {
  function go() {
    $y = $this->fun();
    var_dump($y);
    $y = $this->fun();
    var_dump($y);
  }
}

function main() {
  (new Foo)->go();
}

main();
