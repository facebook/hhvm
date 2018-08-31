<?php

class test {
  function foo() {
    $var = $this->blah->prop->foo->bar = "string";
    var_dump($this->blah);
  }
}

<<__EntryPoint>>
function main_785() {
$t = new test;
$t->foo();
}
