<?php
  //line 681 ...
  $array = array(array(7,8,9),1,2,3,array(4,5,6));
$recursiveArrayIterator = new RecursiveArrayIterator($array);
$test = new RecursiveIteratorIterator($recursiveArrayIterator);

//var_dump($test->current());
$test->setMaxDepth();
$test->setMaxDepth(1);
$test->setMaxDepth(1,2);
$test->setMaxDepth(1,2,3);

//var_dump($test->current());


?>
===DONE===