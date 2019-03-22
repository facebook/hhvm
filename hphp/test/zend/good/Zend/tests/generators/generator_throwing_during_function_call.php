<?php

function throwException() {
    throw new Exception('test');
}

function gen() {
    yield 'foo';
    strlen("foo", "bar", throwException());
    yield 'bar';
}

$gen = gen();
$gen->next();
var_dump($gen->current());

try {
    $gen->next();
} catch (Exception $e) {
    echo 'Caught exception with message "', $e->getMessage(), '"', "\n";
}

var_dump($gen->current());

