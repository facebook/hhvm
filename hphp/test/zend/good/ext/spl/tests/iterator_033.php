<?hh
<<__EntryPoint>> function main(): void {
$it = new ParentIterator(new RecursiveArrayIterator(varray[1,varray[21,22, varray[231]],3]));

foreach(new RecursiveIteratorIterator($it) as $k=>$v)
{
	var_dump($k);
	var_dump($v);
}

echo "==SECOND==\n";

foreach(new RecursiveIteratorIterator($it, 1) as $k=>$v)
{
	var_dump($k);
	var_dump($v);
}

echo "===DONE===\n";
}
