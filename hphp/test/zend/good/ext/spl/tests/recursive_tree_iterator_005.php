<?hh
<<__EntryPoint>> function main(): void {
$ary = darray[
	0 => varray[
		(string)"binary",
		"abc2",
		1,
	],
	(string)"binary" => darray[
		0 => 2,
		1 => "b",
		3 => varray[
			4,
			"c",
		],
		"4abc" => varray[
			4,
			"c",
		],
	],
];

$it = new RecursiveTreeIterator(new RecursiveArrayIterator($ary), 0);
foreach($it as $k => $v) {
	var_dump($v);
}
echo "\n----------------\n\n";
foreach($it as $k => $v) {
	var_dump($k);
}
echo "\n----------------\n\n";
echo "key, getEntry, current:\n";
foreach($it as $k => $v) {
	var_dump($it->key(), $it->getEntry(), $it->current());
}
echo "===DONE===\n";
}
