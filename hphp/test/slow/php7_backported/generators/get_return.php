<?php

function gen1() {
    return 42;
    yield 24;
}

$gen = gen1();
// Calling getReturn() directly here is okay due to auto-priming
var_dump($gen->getReturn());

function gen2() {
    yield 24;
    return 42;
}

$gen = gen2();
var_dump($gen->current());
$gen->next();
var_dump($gen->getReturn());

// ============================================================================
// `gen3` was testing by-reference yields, which PHP 5/7 support but which
// HHVM does not. That test should be added back in when HHVM adds support for
// this feature.
// ============================================================================

// Return types for generators specify the return of the function,
// not of the generator return value, so this code is okay
function gen4() {
    yield 24;
    return 42;
}

$gen = gen4();
var_dump($gen->current());
$gen->next();
var_dump($gen->getReturn());

// Has no explicit return, but implicitly return NULL at the end
function gen5() {
    yield 24;
}

$gen = gen5();
var_dump($gen->current());
$gen->next();
var_dump($gen->getReturn());

// Explicit value-less return also results in a NULL generator
// return value and there is no interference with type hints
function gen6() {
    return;
    yield 24;
}

$gen = gen6();
var_dump($gen->getReturn());
