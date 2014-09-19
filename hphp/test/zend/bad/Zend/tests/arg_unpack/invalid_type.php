<?php

function test(...$args) {
    var_dump($args);
}

test(...null);
test(...42);
test(...new stdClass);

test(1, 2, 3, ..."foo", ...[4, 5]);
test(1, 2, 3, ...new StdClass, ...3.14, ...[4, 5]);

?>
