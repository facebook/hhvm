<?php

class MyPriorityQueue extends \SplPriorityQueue {
  public function __construct() {
    $this->setExtractFlags(self::EXTR_DATA);
  }

  public function sort() {
    $sortedList = array();

    while ($this->valid()) {
      $sortedList[] = $this->extract();
    }

    return array_reverse($sortedList);
  }
}

$data  = array(
  array('test1', 1),
  array('test3', 3),
  array('test2', 2),
);
$queue = new MyPriorityQueue();

foreach ($data as $entry) {
  $queue->insert($entry[0], $entry[1]);
}

var_dump($queue->sort());
