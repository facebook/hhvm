<?php

$array = [1, 2, 3];
foreach ($array as &$ref) {
    /* Free array, causing free of iterator */
    $array = [];
    /* Reuse the iterator.
     * However it will also be reused on next foreach iteration */
    $it = new ArrayIterator([1, 2, 3]);
    $it->rewind();
}
var_dump($it->current());

?>
