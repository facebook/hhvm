<?php

function test(array... $args) {
    var_dump($args);
}

test();
test([0], [1], [2]);
test([0], [1], 2);

?>
