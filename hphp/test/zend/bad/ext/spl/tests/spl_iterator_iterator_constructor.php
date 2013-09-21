<?php

  //I think this is testing line 1297 of spl_iterators.c

  $array = array(array(7,8,9),1,2,3,array(4,5,6));
$arrayIterator = new ArrayIterator($array);
try {
$test = new IteratorIterator($arrayIterator);

$test = new IteratorIterator($arrayIterator, 1);
$test = new IteratorIterator($arrayIterator, 1, 1);
$test = new IteratorIterator($arrayIterator, 1, 1, 1);
$test = new IteratorIterator($arrayIterator, 1, 1, 1, 1);

} catch (InvalidArgumentException $e){
  print  $e->getMessage() . "\n";
}


?>
===DONE===