<?php

function gen() {
    yield;
}
<<__EntryPoint>> function main() {
$gen = gen();
$gen->next();
$gen->throw(new stdClass);
}
