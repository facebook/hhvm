<?php

trait foo {
}

var_dump(trait_exists('foo'));
var_dump(trait_exists(1));
var_dump(trait_exists(NULL));
var_dump(trait_exists(new stdClass));

?>