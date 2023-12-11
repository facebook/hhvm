<?hh
<<__EntryPoint>> function main(): void {
$ar = dict[0 => 0, 1 => "123", 2 => 123, 22 => "abc", 23 => "a2b", 24 => 22, "a2d" => 7, 25 => 42];

foreach(new RegexIterator(new ArrayIterator($ar), "/2/") as $k => $v)
{
	echo "$k=>$v\n";
}

echo "===KEY===\n";

foreach(new RegexIterator(new ArrayIterator($ar), "/2/", 0, RegexIterator::USE_KEY) as $k => $v)
{
	echo "$k=>$v\n";
}

echo "===DONE===\n";
}
