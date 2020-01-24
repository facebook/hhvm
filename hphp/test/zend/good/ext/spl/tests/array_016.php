<?hh
<<__EntryPoint>> function main(): void {
$it = new ArrayIterator(range(0,3));

foreach(new IteratorIterator($it) as $v)
{
	var_dump($v);
}

echo "===DONE===\n";
}
