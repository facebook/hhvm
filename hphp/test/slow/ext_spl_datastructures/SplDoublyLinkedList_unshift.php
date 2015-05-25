<?php

$dll = new SplDoublyLinkedList();

$dll->unshift('foo');
$dll->unshift('bar');

echo $dll->shift(), PHP_EOL;
$dll->rewind();
echo $dll->current(), PHP_EOL;
$dll->prev();
echo $dll->current(), PHP_EOL;
