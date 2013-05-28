<?php

trait T {
  public static $x=1;
  public function printX() {
 var_dump(self::$x);
 }
}
class C1 {
 use T;
 }
class C2 {
 use T;
 }
$o1 = new C1;
$o2 = new C2;
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
T::$x++;
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
C1::$x++;
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
C2::$x++;
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
$o1->x++;
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
