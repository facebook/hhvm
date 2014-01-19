<?php

$ai = new ArrayIterator(array('foo', 'bar'));

$ci = new CachingIterator($ai);
$ci->setFlags(); //expects arg

?>