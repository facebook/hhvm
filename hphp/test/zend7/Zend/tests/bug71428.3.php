<?php
class A { }
class B           {  public function m(A $a = NULL, $n) { echo "B.m";} };
class C extends B {  public function m(A $a       , $n) { echo "C.m";} };
?>
