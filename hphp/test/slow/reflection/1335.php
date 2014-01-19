<?php

interface A {
 function foo();
 }
abstract class B implements A {
 function bar() {
}
}
var_dump(get_class_methods('B'));
