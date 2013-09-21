<?php
$di0 = new DateInterval('P2Y4DT6H8M');

$s = serialize($di0);

$di1 = unserialize($s);

var_dump($di0, $di1);

?>
==DONE==