<?php

function gen() {
    yield;
}

$gen = gen();
$gen->send(NULL);

echo "DONE";

?>