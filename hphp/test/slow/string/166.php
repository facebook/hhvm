<?php

function f() {
 return 'x';
 }
function g() {
}
function test1($a) {
  $buf = '';
  foreach ($a as $s) {
     $buf .= f() . g() . 'h' . f() . 'h' . g();
  }
  foreach ($a as $s) {
    $buf .= ($s . 'h' . $s);
  }
  return $buf;
}
var_dump(test1(array(1)));
function test2() {
  return f() . g() . f() . g();
}
var_dump(test2());
function test3() {
  return f() . g() . f() . g() . f() . g() . f() . g() . f();
}
var_dump(test3());
function test4() {
  $s = f();
  $s .=    ('foo'.    'bar'.    f().    'foo'.    'baz'.    f().    'fuz'.    'boo'.    f().    'fiz'.     'faz');
  $s .= f();
  return $s;
}
var_dump(test4());
function test5() {
  return g().g().g().g();
}
var_dump(test5());
function test6() {
  return g().f().g();
}
var_dump(test6());
