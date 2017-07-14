<?php

test($foo = new stdClass);
var_dump($foo);
test($bar = 2);
var_dump($bar);
test($baz = &$bar);
var_dump($baz);

function test(&$param) {
        $param = 1;
}

?>
