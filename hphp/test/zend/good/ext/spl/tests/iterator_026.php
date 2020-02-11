<?hh
<<__EntryPoint>> function main(): void {
$ar = varray[1, 2, varray[31, 32, varray[331]], 4];

$it = new RecursiveArrayIterator($ar);
$it = new RecursiveCachingIterator($it);
$it = new RecursiveIteratorIterator($it);

foreach($it as $k=>$v)
{
	echo "$k=>$v\n";
	echo "hasNext: " . ($it->getInnerIterator()->hasNext() ? "yes" : "no") . "\n";
}

echo "===DONE===\n";
}
