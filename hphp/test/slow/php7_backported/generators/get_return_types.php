<?php

function gen1() {
    return; // CONST
    yield;
}

$gen = gen1();
var_dump($gen->getReturn());

function gen2() {
    return "str"; // CONST
    yield;
}

$gen = gen2();
var_dump($gen->getReturn());

function gen3($var) {
    return $var; // CV
    yield;
}

$gen = gen3([1, 2, 3]);
var_dump($gen->getReturn());

function gen4($obj) {
    return $obj->prop; // VAR
    yield;
}

$gen = gen4((object) ['prop' => 321]);
var_dump($gen->getReturn());

function gen5($val) {
    return (int) $val; // TMP
    yield;
}

$gen = gen5("42");
var_dump($gen->getReturn());
