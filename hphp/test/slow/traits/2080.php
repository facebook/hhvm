<?php

trait T1 {
 function foo() {
 yield 1;
 }
 }
class C {
 use T1 {
 T1::foo as static;
 }
 }
