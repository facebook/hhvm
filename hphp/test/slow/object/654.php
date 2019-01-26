<?php

$a = 1;
 class A {
 public function t() {
 global $a;
 var_dump($a);
}
}
 $obj = new A();
 $obj->t();
