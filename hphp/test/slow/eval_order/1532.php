<?php

trait T {
}
function __autoload($c) {
  var_dump($c);
  class A {
 use T;
 }
}
var_dump(class_uses("A", false));
var_dump(class_uses("A"));
var_dump(class_exists("A"));
