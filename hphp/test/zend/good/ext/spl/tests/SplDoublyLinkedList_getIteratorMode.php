<?hh <<__EntryPoint>> function main(): void {
$list = new SplDoublyLinkedList();
$list->setIteratorMode(SplDoublyLinkedList::IT_MODE_FIFO | SplDoublyLinkedList::IT_MODE_KEEP);
echo $list->getIteratorMode();
}
