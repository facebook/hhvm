<?php

function fn(...$args) {
    var_dump(count($args));
}

$array = array_fill(0, 10000, 42);
fn(...$array, ...$array);

?>
