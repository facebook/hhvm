<?php

function test() {}
$iterator = new LimitIterator(
    new InfiniteIterator(new ArrayIterator([42])),
    0, 17000
);
test(...$iterator);

?>
===DONE===
