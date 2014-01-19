<?php

abstract class T {
 abstract function test();
 }
 class R extends T {
 function test() {
 var_dump('test');
 }
}
 $obj = 1;
 $obj = new R();
 $obj->test();
