<?php

function iterate_queue_with_mode($mode) {
  $x = new SplQueue();
  $x->setIteratorMode($mode);
  $x->push(1);
  $x->push(2);
  $x->push(3);
  foreach ($x as $y) {
    var_dump($y);
  }
  var_dump(count($x));
}

function main() {
  iterate_queue_with_mode(SplQueue::IT_MODE_FIFO);
  iterate_queue_with_mode(SplQueue::IT_MODE_FIFO | SplQueue::IT_MODE_DELETE);
  iterate_queue_with_mode(SplQueue::IT_MODE_DELETE);

  // Should fail
  iterate_queue_with_mode(SplQueue::IT_MODE_LIFO);
}

main();
