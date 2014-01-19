<?php

trait Too {
  function gen() {
    $abc = $this->input();
    $a = function ($arg) use ($abc) {
      var_dump($arg);
      var_dump($abc);
      return $this->output();
    }
;
    return $a;
  }
  function input() {
 return 1;
 }
  function output() {
 return 2;
 }
}
class Foo {
  use Too;
  function input() {
 return "str1";
 }
  function output() {
 return "str2";
 }
}
class Goo {
  use Too;
  function input() {
 return false;
 }
  function output() {
 return true;
 }
}
$of = new Foo;
$f = $of->gen();
var_dump($f(1000));
$og = new Goo;
$g = $og->gen();
var_dump($g(2000));
