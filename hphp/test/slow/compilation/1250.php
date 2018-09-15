<?php

class A {
 public $a = array('t' => 't');
}
 class B {
 public $a;
}

 <<__EntryPoint>>
function main_1250() {
$a = 1;
 $a = new A();
 $a->a['t'] = true;
 var_dump($a->a['t']);
}
