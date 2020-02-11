<?hh <<__EntryPoint>> function main(): void {
$a = varray["some","blah"];
$i = new ArrayIterator($a);

$ci = new CachingIterator($i);

$ci->rewind();

echo "===DONE===\n";
}
