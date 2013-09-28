<?php
$iterator = new RecursiveIteratorIterator(new RecursiveArrayIterator(array()));
$iterator->getSubIterator();
$iterator->getSubIterator(0);
$iterator->getSubIterator(0, 0);
?>