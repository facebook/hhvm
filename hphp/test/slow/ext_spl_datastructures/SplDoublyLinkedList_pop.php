<?hh


<<__EntryPoint>>
function main_spl_doubly_linked_list_pop() :mixed{
$dll = new SplDoublyLinkedList();

$dll->push('foo');
$dll->push('bar');

echo $dll->pop(), PHP_EOL;
$dll->rewind();
echo $dll->current(), PHP_EOL;
$dll->next();
echo $dll->current(), PHP_EOL;
}
