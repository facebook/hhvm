<?php

function test1(... $args) {
    var_dump($args);
}

test1();
test1(1);
test1(1, 2, 3);

function test2($arg1, $arg2, ...$args) {
    var_dump($arg1, $arg2, $args);
}

test2(1, 2);
test2(1, 2, 3);
test2(1, 2, 3, 4, 5);

?>
