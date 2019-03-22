<?php

function gen() {
    yield;
}

$gen = gen();
$gen->next();
$gen->throw(new stdClass);

