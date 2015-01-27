<?php

$fn = function(...$args) {
    var_dump($args);
};

$fn(...[]);
$fn(...[1, 2, 3]);
$fn(1, ...[2, 3], ...[], ...[4, 5]);

?>
