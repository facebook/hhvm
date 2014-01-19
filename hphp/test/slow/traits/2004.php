<?php

trait T1 {
 function F() {
}
 }
trait T2 {
 function F() {
}
 }
trait T3 {
 use T2 {
 F as G;
 }
 }
class C1 {
  use T1, T2 {
    T1::F insteadof T2;
    T2::F as G;
  }
}
class C2 {
  use T3;
}
$rc1 = new ReflectionClass('C1');
var_dump($rc1->getTraitAliases());
$rc2 = new ReflectionClass('C2');
var_dump($rc2->getTraitAliases());
$rc3 = new ReflectionClass('T3');
var_dump($rc3->getTraitAliases());
