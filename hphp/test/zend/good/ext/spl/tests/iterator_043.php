<?hh
<<__EntryPoint>> function main(): void {
$it = new RecursiveCachingIterator(new RecursiveArrayIterator(varray[1,2]));

var_dump($it->getChildren());
$it->rewind();
var_dump($it->getChildren());

echo "===DONE===\n";
}
