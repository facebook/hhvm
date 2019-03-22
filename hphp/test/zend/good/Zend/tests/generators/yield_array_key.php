<?php

function gen() {
    yield [] => 1;
}

$gen = gen();
$gen->next();
var_dump($gen->key());
var_dump($gen->current());

