<?php

$e = new Exception('aaa', 200);

$a = serialize($e);

$b = unserialize($a);

var_dump($b->__toString());
?>
