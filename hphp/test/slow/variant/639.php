<?php

class a {
 public $var2 = 1;
 public $var1;
 }
class b extends a {
 public $var2;
 }
function f() {
 $obj1 = new b();
 var_dump($obj1);
 $obj1->var1 = 1;
 }
f();
