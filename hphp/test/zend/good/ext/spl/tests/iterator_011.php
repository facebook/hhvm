<?hh
<<__EntryPoint>> function main(): void {
echo "===EmptyIterator===\n";

foreach(new LimitIterator(new InfiniteIterator(new EmptyIterator()), 0, 3) as $key=>$val)
{
	echo "$key=>$val\n";
}

echo "===InfiniteIterator===\n";

$it = new ArrayIterator(darray[0 => 'A', 1 => 'B', 2 => 'C', 3 => 'D']);
$it = new InfiniteIterator($it);
$it = new LimitIterator($it, 2, 5);
foreach($it as $val=>$key)
{
	echo "$val=>$key\n";
}

echo "===Infinite/LimitIterator===\n";

$it = new ArrayIterator(darray[0 => 'A', 1 => 'B', 2 => 'C', 3 => 'D']);
$it = new LimitIterator($it, 1, 2);
$it = new InfiniteIterator($it);
$it = new LimitIterator($it, 2, 5);
foreach($it as $val=>$key)
{
	echo "$val=>$key\n";
}

echo "===DONE===\n";
}
