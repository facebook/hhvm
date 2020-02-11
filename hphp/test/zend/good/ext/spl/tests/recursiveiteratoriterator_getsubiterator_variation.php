<?hh <<__EntryPoint>> function main(): void {
$sample_array = varray[1, 2, varray[3, 4]];

$iterator = new RecursiveIteratorIterator(new RecursiveArrayIterator($sample_array));

$iterator->next();
$iterator->next();
var_dump(get_class($iterator->getSubIterator()));
var_dump($iterator->getSubIterator()->getArrayCopy());
$iterator->next();
var_dump(get_class($iterator->getSubIterator()));
var_dump($iterator->getSubIterator()->getArrayCopy());
}
