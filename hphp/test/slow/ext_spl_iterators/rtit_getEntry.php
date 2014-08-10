<?php

$rait = new RecursiveArrayIterator([0,1,[2,3,[4,5]]]);

$rtit = new RecursiveTreeIterator($rait);

foreach($rtit as $key=>$val) {
  var_dump($rtit->getEntry());
}
