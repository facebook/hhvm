<?php

function gen() {
    yield [] => 1;
}

$gen = gen();
var_dump($gen->key());
var_dump($gen->current());

?>