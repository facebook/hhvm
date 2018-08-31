<?php

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
{

  function callGetChildren() {
    return new StdClass();
  }
}


<<__EntryPoint>>
function main_unexpected_value_exception() {
try {
  $arr = array(0, array(1));
  foreach(new RecursiveArrayIteratorIterator(
          new RecursiveArrayIterator($arr)) as $k=>$v) {
  }
} catch(UnexpectedValueException $e) {
  echo "UnexpectedValueException caught", PHP_EOL;
}
}
