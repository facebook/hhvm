<?php

if (!isset($h)) {
  if (isset($g)) {
    interface A {
 function foo();
 }
  }
 else {
    interface A {
 function foo();
 }
  }
}
 else {
  if (isset($g)) {
    interface X {
 function foo();
 }
  }
 else {
    interface X {
 function foo();
 }
  }
}
abstract class B implements A {
 function bar() {
}
 }
var_dump(get_class_methods('A'));
var_dump(get_class_methods('B'));
var_dump(get_class_methods('X'));
var_dump(get_class_methods('Y'));
