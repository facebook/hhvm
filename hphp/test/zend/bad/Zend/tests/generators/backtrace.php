<?php

function f1() {
    debug_print_backtrace();
}

function f2($arg1, $arg2) {
    f1();
    yield; // force generator
}

function f3($gen) {
    $gen->rewind(); // trigger run
}

$gen = f2('foo', 'bar');
f3($gen);

?>