<?php

function foo() {
    return "bar";
}

function &gen() {
    yield foo();
}

$gen = gen();
var_dump($gen->current());

?>