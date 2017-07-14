<?php

class A {
    public $b;
}

$arr = [];

$a = new A;
$a->b =& $arr;

(new ReflectionProperty('A', 'b'))->getValue($a)[] = 42;

var_dump($a);

?>
