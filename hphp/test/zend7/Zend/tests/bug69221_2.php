<?php

$gen = function() {
    yield 1;
};

$iter = new IteratorIterator($gen());
$ngen = $iter->getInnerIterator();

var_dump(iterator_to_array($ngen, false));

?>
