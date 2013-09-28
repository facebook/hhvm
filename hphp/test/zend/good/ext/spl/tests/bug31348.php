<?php
$a = Array("some","blah");
$i = new ArrayIterator($a);

$ci = new CachingIterator($i);

$ci->rewind();

?>
===DONE===