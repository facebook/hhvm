<?php

class A {
    public $test = false;
    public $test2 = '';
    public $test3 = array();
    public $_ = false;
}


<<__EntryPoint>>
function main_mb_convert_variables_empty() {
$a = new A();
mb_convert_variables('utf-8', 'windows-1251', $a);
var_dump($a);

$a = array('test' => array());
mb_convert_variables('utf-8', 'windows-1251', $a);
var_dump($a);
}
