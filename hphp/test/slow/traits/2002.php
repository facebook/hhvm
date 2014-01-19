<?php

trait T2 {
}
trait T3 {
 use T2;
 }
trait T1 {
}
class C1 {
 }
class C2 {
 use T2;
 }
class C3 {
 use T3, T1;
 }
var_dump(class_uses(new C1));
var_dump(class_uses(new C2));
var_dump(class_uses(new C3));
