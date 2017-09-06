<?php

$x = gmp_init(99);
$x->foo = 'lol';
$x->bar = 'wut';
var_dump($x);
var_dump(unserialize(serialize($x)));
