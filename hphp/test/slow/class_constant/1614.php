<?php

interface I {
 const X = 'x';
 }
class A implements I {
 }
var_dump(A::X);
var_dump(get_class_constants('A'));
