<?php

class TestHeap extends SplHeap {

  function compare() {
    print "This shouldn't be printed";
  }
}

$testHeap = new TestHeap();



var_dump($testHeap->extract('test'));

?>
===DONE===