<?php

class A {
 var $num;
 }
function foo() {
 return new A();
 }

<<__EntryPoint>>
function main_775() {
foreach(($a=(object)new A()) as $v);
foreach(($a=(object)foo()) as $v);
foreach(($a=new A()) as $v);
$a->num = 1;
print($a->num);
}
