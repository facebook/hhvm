<?php
$q = new SplQueue();

$q->enqueue("a");
$q->enqueue("b");

var_dump($q, $ss = serialize($q), unserialize($ss));

$s = new SplStack();

$s->push("a");
$s->push("b");

var_dump($s, $ss = serialize($s), unserialize($ss));
?>
==END==