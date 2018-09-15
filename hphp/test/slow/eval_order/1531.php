<?php

class B {
}
function __autoload($c) {
  var_dump($c);
  class A extends B {
}
}

<<__EntryPoint>>
function main_1531() {
var_dump(class_parents("A", false));
var_dump(class_parents("A"));
var_dump(class_exists("A"));
}
