<?hh
function test(SplDoublyLinkedList $l) :mixed{
  $l->push("a");
  $l->push("b");
  $l->push("c");

  echo "Foreach:", PHP_EOL;
  foreach ($l as $key=>$val) {
    echo $key, '=>', $val, PHP_EOL;
  }
  echo PHP_EOL;

  echo "Foreach:", PHP_EOL;
  foreach ($l as $key=>$val) {
    echo $key, '=>', $val, PHP_EOL;
  }
  echo PHP_EOL;
}


<<__EntryPoint>>
function main_doubly_linked_list_it_mode() :mixed{
echo "FIFO:", PHP_EOL;
test(new SplDoublyLinkedList());

$list = new SplDoublyLinkedList();
$list->setIteratorMode(SplDoublyLinkedList::IT_MODE_LIFO);

echo "-------------------------", PHP_EOL;
echo "LIFO:", PHP_EOL;
test($list);
}
