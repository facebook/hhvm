<?php


$testHeap = new SplPriorityQueue();


var_dump($testHeap->insert());
var_dump($testHeap->insert('test'));
var_dump($testHeap->insert('test', 'test'));
var_dump($testHeap->insert('test', 'test', 'test'));


?>
===DONE===