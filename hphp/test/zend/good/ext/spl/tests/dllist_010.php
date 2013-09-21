<?php
$dll = new SplDoublyLinkedList();
$dll->push(1);
$dll->push(2);
$dll->push(3);
$dll->push(4);


$dll->rewind();
$dll->prev();
var_dump($dll->current());
$dll->rewind();
var_dump($dll->current());
$dll->next();
var_dump($dll->current());
$dll->next();
$dll->next();
var_dump($dll->current());
$dll->prev();
var_dump($dll->current());

?>
===DONE===