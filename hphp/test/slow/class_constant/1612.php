<?php

function __autoload($x) {
 var_dump('AUTOLOAD:'.$x);
 }
class X {
  public $foo = Y::FOO;
  function foo() {
    var_dump(__METHOD__, $this);
  }
}

<<__EntryPoint>>
function main_1612() {
X::foo();
}
