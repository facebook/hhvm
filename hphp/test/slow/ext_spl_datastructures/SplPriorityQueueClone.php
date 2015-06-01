<?php

function getQueueWithLittleData() {
  $q = new SplPriorityQueue();
  $q->insert('dux', 4);
  $q->insert('legati', 3);
  $q->insert('centurion', 2);
  $q->insert('munifex', 1);
  return $q;
}

function testExtractFlags() {
  $flags = array(
    array('SplPriorityQueue::EXTR_DATA', SplPriorityQueue::EXTR_DATA),
    array('SplPriorityQueue::EXTR_PRIORITY', SplPriorityQueue::EXTR_PRIORITY),
    array('SplPriorityQueue::EXTR_BOTH', SplPriorityQueue::EXTR_BOTH),
  );
  $sources = array(
    getQueueWithLittleData(),
    clone getQueueWithLittleData(),
  );
  foreach ($sources as $queue) {
    foreach ($flags as $flagInfo) {
      list($name, $value) = $flagInfo;
      testExtractFlag($queue, $name, $value);
    }
  }
}

function testExtractFlag($q, $flagName, $flagValue) {
  $q->setExtractFlags($flagValue);
  echo "Get top with $flagName:\n";
  print_r($q->top());
  echo "\n";

  $q->rewind();
  $q->next();
  $q->valid();

  echo "Second in rank with $flagName:\n";
  print_r($q->current());
  echo "\n";

  echo "\n";
}
testExtractFlags();


function testCloneGivesValidCopy() {
  $q = getQueueWithLittleData();
  $clonedQueue = clone $q;

  $clonedQueue->top();

  echo "Top rank (even after killing the top in a clone) is: ";
  echo $q->top() . "\n";
}
testCloneGivesValidCopy();
