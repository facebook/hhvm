<?php
function foo() {
    throw new Exception;
    yield 1;
    yield 2;
    return 42;
}

$bar = foo();

try {
    $bar->next();
} catch (Exception $e) {
}

var_dump($bar->getReturn());
