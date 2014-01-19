<?php

class A {
 function foo() {
}
 }
class B extends A {
 function bar() {
}
}
var_dump(get_class_methods(new B()));
