<?php

class X {
public $a = 3;
function foo() {
$thiz =& $this;
$thiz = 5;
var_dump($this->a);
var_dump($this);
}
}

<<__EntryPoint>>
function main_799() {
$x = new X;
$x->foo();
}
