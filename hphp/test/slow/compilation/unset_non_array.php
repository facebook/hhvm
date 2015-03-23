<?php
function test(SplDoublyLinkedList $l) {
  $l->push("a");
  $l->push("b");
  $l->push("c");

  echo "ArrayAccess Unset:", PHP_EOL;
  unset($l[0]);
  var_dump($l[0]);
  echo PHP_EOL;
}

echo "FIFO:", PHP_EOL;
test(new SplDoublyLinkedList());

$list = new SplDoublyLinkedList();
$list->setIteratorMode(SplDoublyLinkedList::IT_MODE_LIFO);

echo "-------------------------", PHP_EOL;
echo "LIFO:", PHP_EOL;
test($list);
