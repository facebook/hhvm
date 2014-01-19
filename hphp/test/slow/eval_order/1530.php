<?php

interface I {
}
function __autoload($c) {
  var_dump($c);
  class A implements I {
}
}
var_dump(class_implements("A", false));
var_dump(class_implements("A"));
var_dump(class_exists("A"));
