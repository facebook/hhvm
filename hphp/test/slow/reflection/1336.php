<?php

interface I1 {
 function ifoo2();
 function ifoo1();
 }
interface I2 {
 function ifoo4();
 function ifoo3();
 }
class A {
 function foo() {
}
 function foo2() {
}
 }
abstract class B extends A implements I1, I2 {
 function bar() {
}
}
abstract class C extends A implements I2, I1 {
 function bar() {
}
}
class D extends C {
 function ifoo2() {
}
 function ifoo1() {
}
  function ifoo4() {
}
 function ifoo3() {
}
 function bar() {
}
 }
var_dump(get_class_methods('B'));
var_dump(get_class_methods('C'));
