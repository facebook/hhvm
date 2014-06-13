<?php

class RecItIt extends RecursiveIteratorIterator {

  public function next() {
    echo "Called next() \n";
    parent::next();
  }

  public function rewind() {
    echo "Called rewind() \n";
    parent::rewind();
  }

}


$rii = new RecItIt(new RecursiveArrayIterator(array(1,2,3,4)));
$rii->rewind();
