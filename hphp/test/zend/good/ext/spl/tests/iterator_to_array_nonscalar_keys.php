<?php

function gen() {
    yield "foo" => 0;
    yield 1     => 1;
    yield 2.5   => 2;
    yield null  => 3;
    yield []    => 4;
    yield new stdClass => 5;
}

var_dump(iterator_to_array(gen()));

?>