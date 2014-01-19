<?php
$h = new RecursiveArrayIterator(array());
$x = new reflectionmethod('RecursiveArrayIterator', 'asort');
$z = $x->invoke($h);
?>
DONE