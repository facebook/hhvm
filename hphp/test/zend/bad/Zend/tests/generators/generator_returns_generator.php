<?php

function gen() {
    // execution is suspended here, so the following never gets run:
    echo "Foo";
	// trigger a generator
    yield;
}

$generator = gen();
var_dump($generator instanceof Generator);

?>