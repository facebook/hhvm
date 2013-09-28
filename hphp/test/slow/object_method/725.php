<?php

abstract class T {
 abstract function test();
 function foo() {
 $this->test();
}
 }
class R extends T {
 function test() {
 var_dump('test');
 }
}
 $obj = new R();
 $obj->test();
 $obj->foo();
