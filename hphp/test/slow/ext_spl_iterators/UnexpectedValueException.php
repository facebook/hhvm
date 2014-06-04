<?php

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
{

  function callGetChildren() {
    return new StdClass();
  }
}

try {
  $arr = array(0, array(1));
  foreach(new RecursiveArrayIteratorIterator(
          new RecursiveArrayIterator($arr)) as $k=>$v) {
  }
} catch(UnexpectedValueException $e) {
  echo "UnexpectedValueException caught", PHP_EOL;
}
