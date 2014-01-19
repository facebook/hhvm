<?php

interface foo {
}

var_dump(interface_exists('foo'));
var_dump(interface_exists(1));
var_dump(interface_exists(NULL));
var_dump(interface_exists(new stdClass));

?>