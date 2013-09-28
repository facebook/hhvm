<?php

abstract class T {
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
