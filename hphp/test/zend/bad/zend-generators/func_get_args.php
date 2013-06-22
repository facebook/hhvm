<?php

function gen() {
    var_dump(func_get_args());
    yield; // trigger generator
}

$gen = gen("foo", "bar");
$gen->rewind();

?>