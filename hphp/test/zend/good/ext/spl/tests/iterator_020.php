<?hh
<<__EntryPoint>> function main(): void {
echo "===Empty===\n";

$it = new AppendIterator;

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===Append===\n";

$it->append(new ArrayIterator(darray[0 => 'A', 1 => 'B']));

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===Rewind===\n";

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===Append===\n";

$it->append(new ArrayIterator(darray[2 => 'C', 3 => 'D']));

foreach(new NoRewindIterator($it) as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===Rewind===\n";

foreach($it as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===DONE===\n";
}
