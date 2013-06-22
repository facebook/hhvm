<?php

function gen() {
    yield;
}

$gen = gen();
clone $gen;

?>