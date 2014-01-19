<?php

function gen() {
    yield;
}

$gen = gen();
$gen->throw(new stdClass);

?>