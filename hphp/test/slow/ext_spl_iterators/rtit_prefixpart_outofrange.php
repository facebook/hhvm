<?php

$rait = new RecursiveArrayIterator([0,1,[2,3]]);
$rtit = new RecursiveTreeIterator($rait);

try {
  $rtit->setPrefixPart(6, "");
} catch (OutOfRangeException $e) {
  echo $e->getMessage(), PHP_EOL;
}
