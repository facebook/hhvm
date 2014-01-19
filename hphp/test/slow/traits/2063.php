<?php

trait T {
 private $x = 0;
}
class X {
 use T;
 function x() {
 return ++$this->x;
 }
 }
class Y extends X {
 use T;
 function y() {
 return ++$this->x;
 }
 }
class Z extends Y {
 function z() {
 return ++$this->x;
 }
 }
$a = new Z();
echo join(" ", array($a->x(), $a->x(), $a->y(), $a->y(), $a->z(), $a->z())), "\n";
