<?php

function gen() {
    fn();
    yield;
}

function fn() {
    exit('Done');
}

$gen = gen();
$gen->current();

?>