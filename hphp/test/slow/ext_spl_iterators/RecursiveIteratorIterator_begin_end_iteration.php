<?php

class rii extends RecursiveIteratorIterator
{
  function rewind() {
    echo __METHOD__ . "()\n";
    parent::rewind();
  }

  function beginIteration()
  {
    echo __METHOD__ . "()\n";
  }

  function endIteration()
  {
    echo __METHOD__ . "()\n";
  }

  function valid() {
    echo __METHOD__ . "()\n";
    return parent::valid();
  }
}

$ar = array(1, 2, array(31));

$it = new rii(new ArrayObject($ar, 0, "RecursiveArrayIterator"));

$it->rewind();
$it->rewind();
$it->rewind();

while($it->valid()) // call EndIteration on last call
  $it->next();

$it->valid(); // Don't call EndIteration
$it->valid(); // Don't
