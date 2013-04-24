<?php

$it = new AppendIterator();
$it->append(new ArrayIterator(array(1,2)));
$it->append(new ArrayIterator(array(2,3)));

var_dump(iterator_to_array($it));
var_dump(iterator_to_array($it, false));
var_dump(iterator_to_array($it, true));

?>
===DONE===