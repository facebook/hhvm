<?hh
<<__EntryPoint>> function main(): void {
$arr = darray[
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

$it = new RecursiveArrayIterator($arr);
$it = new RecursiveTreeIterator($it);

echo "----\n";
echo $it->getPostfix();
echo "\n\n";

echo "----\n";
$it->setPostfix("POSTFIX");
echo $it->getPostfix();
echo "\n\n";

echo "----\n";
foreach($it as $k => $v) {
	echo "[$k] => $v\n";
}

echo "----\n";
$it->setPostfix("");
echo $it->getPostfix();
echo "\n\n";

echo "----\n";
foreach($it as $k => $v) {
	echo "[$k] => $v\n";
}



echo "===DONE===\n";
}
