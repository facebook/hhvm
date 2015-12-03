<?php

function gen1() {
    yield 1;
    yield 2;
    yield 3;
    return 42;
}

function gen2() {
    yield from gen1();
}

$g = gen2();
foreach($g as $val) { var_dump($val); }
var_dump($g->getReturn());
