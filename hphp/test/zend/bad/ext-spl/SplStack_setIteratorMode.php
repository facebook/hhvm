<?php
$stack = new SplStack();
try {
  $stack->setIteratorMode(SplDoublyLinkedList::IT_MODE_FIFO);
} catch (Exception $e) {
  echo $e->getMessage();
}
?>