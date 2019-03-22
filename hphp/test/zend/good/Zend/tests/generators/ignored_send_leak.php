<?php

function gen() {
    yield;
}

$gen = gen();
$gen->next();
$gen->send(NULL);

echo "DONE";

