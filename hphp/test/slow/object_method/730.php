<?php

class c {
function foo() {
 echo "called
";
 }
}
function meh() {
}
function z() {
  $p = new c;
  $p->foo(meh());
  $p = null;
}
z();
