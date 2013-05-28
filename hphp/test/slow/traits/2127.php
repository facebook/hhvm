<?php

if (true) {
  trait T {
 public static function foo() {
 echo "Foo\n";
 }
 }
  class C {
 use T;
 }
}
C::foo();
