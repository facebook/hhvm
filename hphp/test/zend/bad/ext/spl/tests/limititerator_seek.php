<?php

$a = array(1,2,3);
$lt = new LimitIterator(new ArrayIterator($a));

$lt->seek(1,1); // Should throw a warning as seek expects only 1 argument

?>