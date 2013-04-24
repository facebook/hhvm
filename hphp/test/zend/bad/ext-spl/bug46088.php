<?php

$x = new RegexIterator(new ArrayIterator(range(1, 10)), '/\d/');
var_dump($x->accept());

?>