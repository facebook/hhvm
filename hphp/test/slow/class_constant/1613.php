<?php

class A {
 const A = 'a';
 }
class B extends A {
 const B = 'b';
 }
var_dump(get_class_constants('B'));
