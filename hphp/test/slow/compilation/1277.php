<?php

class A {
 const C = 123;
 static function t($a = B::C) {
}
 }
 A::t();
class B {
 const C = 456;
 static function t($a = A::C) {
}
 }
 B::t();
