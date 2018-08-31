<?php

class A {
  public $x;
}

<<__EntryPoint>>
function main_empty_new() {
error_reporting(-1);

var_dump(empty(new A()));
}
