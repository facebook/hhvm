<?hh <<__EntryPoint>> function main(): void {
$ll = new SplDoublyLinkedList();

$ll->push('1');
$ll->push('2');
$ll->push('3');

try {

$ll->offsetUnset($ll->count() + 1);

var_dump($ll);

} catch(Exception $e) {
echo $e->getMessage();
}
}
