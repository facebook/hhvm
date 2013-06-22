<?php

function gen() {
    $gen = yield;
    $gen->next();
}

$gen = gen();
$gen->send($gen);
$gen->next();

?>