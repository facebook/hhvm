<?php

$a = new AppendIterator();
$a1 = new ArrayIterator([1,2,3]);
$a1->next();
$a->append($a1);

var_dump($a->current());
