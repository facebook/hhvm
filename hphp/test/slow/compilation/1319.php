<?php

class X {
 const FOO = 'hello';
 }
function foo(&$a) {
 static $s;
 }
if (class_exists('X')) foo(X::FOO);
