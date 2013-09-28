<?php

function gen() {
    yield 'foo';
    throw new Exception('test');
    yield 'bar';
}

$gen = gen();

var_dump($gen->current());

try {
    $gen->next();
} catch (Exception $e) {
    echo 'Caught exception with message "', $e->getMessage(), '"', "\n";
}

var_dump($gen->current());

?>