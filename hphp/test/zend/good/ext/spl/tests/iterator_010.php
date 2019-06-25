<?hh
<<__EntryPoint>> function main(): void {
echo "===EmptyIterator===\n";

foreach(new LimitIterator(new EmptyIterator(), 0, 3) as $key => $val)
{
	echo "$key=>$val\n";
}

echo "===DONE===\n";
}
