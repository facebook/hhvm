<?php

class test {
  function foo() {
    $var = $this->blah->prop->foo->bar = "string";
    var_dump($this->blah);
  }
}
$t = new test;
$t->foo();
