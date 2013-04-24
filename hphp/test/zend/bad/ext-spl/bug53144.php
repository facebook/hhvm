<?php

$o1 = new StdClass;
$o2 = new StdClass;

$b = new SplObjectStorage();
$b[$o1] = "bar";
$b[$o2] = "baz";

var_dump(count($b));
$b->removeAll($b);
var_dump(count($b));

?>