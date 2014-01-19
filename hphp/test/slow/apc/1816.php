<?php

class A {
 var $a, $b;
 }
$a = new A();
 $a->a = 5;
 $a->b = &$a->a;
apc_store('key', $a);
var_dump(apc_fetch('key'));
