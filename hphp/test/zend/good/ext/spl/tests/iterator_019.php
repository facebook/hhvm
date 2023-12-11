<?hh
<<__EntryPoint>> function main(): void {
echo "===Current===\n";

$it = new NoRewindIterator(new ArrayIterator(dict[0 => 'A', 1 => 'B', 2 => 'C']));

echo $it->key() . '=>' . $it->current() . "\n";

echo "===Next===\n";

$it->next();

echo "===Foreach===\n";

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===DONE===\n";
}
