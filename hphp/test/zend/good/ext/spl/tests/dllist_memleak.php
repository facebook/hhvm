<?hh <<__EntryPoint>> function main(): void {
$dll = new SplDoublyLinkedList();
$dll->push(1);
$dll->push(2);
$dll->push(3);
$dll->push(4);


$dll->rewind();
echo $dll->current()."\n";
$dll->next();
$dll->next();
echo $dll->current()."\n";

echo "===DONE===\n";
}
