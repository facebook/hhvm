<?hh
<<__EntryPoint>> function main(): void {
$ary = darray[
	0 => varray[
		"a",
		1,
	],
	"a" => darray[
		0 => 2,
		1 => "b",
		3 => varray[
			4,
			"c",
		],
		"3" => varray[
			4,
			"c",
		],
	],
];

$it = new RecursiveTreeIterator(new RecursiveArrayIterator($ary));
foreach($it as $k => $v) {
	echo '[' . $it->key() . '] => ' . $it->getPrefix() . $it->getEntry() . $it->getPostfix() . "\n";
}
echo "===DONE===\n";
}
