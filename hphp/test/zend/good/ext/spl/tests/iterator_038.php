<?hh
<<__EntryPoint>> function main(): void {
foreach(new NoRewindIterator(new ArrayIterator(darray['Hello'=>0, 'World'=>1])) as $k => $v)
{
	var_dump($v);
	var_dump($k);
}

echo "===DONE===\n";
}
