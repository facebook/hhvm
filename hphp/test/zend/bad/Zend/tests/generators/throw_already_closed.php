<?php

function gen() {
    yield;
}

$gen = gen();
$gen->next();
$gen->next();
var_dump($gen->valid());
$gen->throw(new Exception('test'));

?>