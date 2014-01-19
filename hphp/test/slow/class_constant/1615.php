<?php

class B {
 const X = 'old';
 }
class A extends B {
 const X = 'new';
 }
var_dump(A::X);
var_dump(get_class_constants('A'));
