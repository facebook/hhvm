<?php

function crash() {
    sin(...[0]);
    throw new \Exception();
    yield;
}

iterator_to_array(crash());

?>
