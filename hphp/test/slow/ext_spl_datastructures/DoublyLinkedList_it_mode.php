<?php
function test(SplDoublyLinkedList $l) {
  $l->push("a");
  $l->push("b");
  $l->push("c");

  echo "Foreach:", PHP_EOL;
  foreach ($l as $key=>$val) {
    echo $key, '=>', $val, PHP_EOL;
  }
  echo PHP_EOL;

  echo "ArrayAccess:", PHP_EOL;
  var_dump($l[0]);
  var_dump($l[1]);
  var_dump($l[2]);
  echo PHP_EOL;

  echo "ArrayAccess Set:", PHP_EOL;
  $l[2] = "two";
  var_dump($l[2]);
  try {
    $l[3] = "five"; // 3 would be the next element
  } catch (OutOfRangeException $e) {
    echo "OutOfRangeException caught", PHP_EOL;
  }

  echo PHP_EOL;

  echo "ArrayAccess Exists:", PHP_EOL;
  var_dump(isset($l[0]), isset($l[2]), isset($l[3])); // true, true, false
  echo PHP_EOL;

  echo "ArrayAccess Unset:", PHP_EOL;
  unset($l[0]);
  var_dump($l->offsetGet(0));
  echo PHP_EOL;

  echo "Foreach:", PHP_EOL;
  foreach ($l as $key=>$val) {
    echo $key, '=>', $val, PHP_EOL;
  }
  echo PHP_EOL;
}

echo "FIFO:", PHP_EOL;
test(new SplDoublyLinkedList());

$list = new SplDoublyLinkedList();
$list->setIteratorMode(SplDoublyLinkedList::IT_MODE_LIFO);

echo "-------------------------", PHP_EOL;
echo "LIFO:", PHP_EOL;
test($list);
