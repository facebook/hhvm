<?php

class A {
  function __construct($a) {
 echo "A\n";
 }
  function __destruct() {
 var_dump($this);
 }
}
function f() {
 echo "f\n";
 throw new Exception();
 }
function test() {
 $a = new A(f());
 }

<<__EntryPoint>>
function main_781() {
try {
 test();
 }
 catch (Exception $e) {
 }
}
