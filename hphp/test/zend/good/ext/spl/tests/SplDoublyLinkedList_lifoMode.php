<?hh <<__EntryPoint>> function main(): void {
$list = new SplDoublyLinkedList();

$list->push('o');
$list->push('o');
$list->push('f');

$list->setIteratorMode(SplDoublyLinkedList::IT_MODE_LIFO);

$list->rewind();

$tmp = $list->current();
while ($tmp) {
  echo $tmp;
  $list->next();
  $tmp = $list->current();
}
}
