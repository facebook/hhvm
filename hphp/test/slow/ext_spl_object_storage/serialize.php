<?php

$s = new SplObjectStorage();

$o1 = new StdClass;
$o2 = new StdClass;

$s->attach($o1, "d1");
$s->attach($o2, "d2");

$serialized = $s->serialize();

$s2 = new SplObjectStorage();
$s2->unserialize($serialized);

var_dump($s == $s2);
?>
