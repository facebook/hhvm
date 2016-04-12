<?php

// TODO: HHVM doesn't support returning from a finally block, which `gen1` was
// testing. It has been removed in the meantime, but should be added back in
// when HHVM adds support for this feature.

function gen2() {
    try {
        return 42;
    } finally {
        throw new Exception("gen2() throw");
    }
    yield;
}

$gen = gen2();
try {
    // This will throw an exception (from the finally)
    // during auto-priming, so fails
    // TODO: Remove this next when HHVM supports auto-priming
    $gen->next();
    var_dump($gen->getReturn());
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}
try {
    // This fails, because the return value was discarded
    var_dump($gen->getReturn());
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}
