<?hh <<__EntryPoint>> function main() {
$dll = new SplDoublyLinkedList();
$dll->rewind();
$dll->prev();
var_dump($dll->current());
echo "===DONE===\n";
}
