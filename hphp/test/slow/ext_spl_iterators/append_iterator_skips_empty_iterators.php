<?php

$a = new AppendIterator();
$a1 = new ArrayIterator([]);
$a2 = new ArrayIterator([1,2,3]);
$a->append($a1);
$a->append($a2);

var_dump($a->valid());
var_dump($a->current());
var_dump($a->getIteratorIndex());

// go to the end
for ($a->rewind(); $a->valid(); $a->next()) {}

$a->append(new ArrayIterator([]));
$a->append(new ArrayIterator([4,5,6]));

var_dump($a->valid());
var_dump($a->current());
var_dump($a->getIteratorIndex());
