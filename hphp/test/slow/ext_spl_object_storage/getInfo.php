<?php
// Test code from: http://www.php.net/manual/en/splobjectstorage.getinfo.php

$s = new SplObjectStorage();

$o1 = new StdClass;
$o2 = new StdClass;

$s->attach($o1, "d1");
$s->attach($o2, "d2");

$s->rewind();
while($s->valid()) {
    $index  = $s->key();
    $object = $s->current(); // similar to current($s)
    $data   = $s->getInfo();

    var_dump($object);
    var_dump($data);
    $s->next();
}

// now mutate $o2 and ensure it sticks
$s->attach($o2, "mutated");
var_dump($s[$o2]);
?>
