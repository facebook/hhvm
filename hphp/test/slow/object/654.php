<?php

$a = 1;
 class A {
 public function t() {
 global $a;
 $b = 'a';
 var_dump($$b);
}
}
 $obj = new A();
 $obj->t();
