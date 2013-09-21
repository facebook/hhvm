<?php

function &gen() {
    yield "foo";
}

$gen = gen();
var_dump($gen->current());

?>