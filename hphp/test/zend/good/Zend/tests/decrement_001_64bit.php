<?hh
<<__EntryPoint>> function main(): void {
$a = varray[
	varray[1,2,3],
	"",
	1,
	2.5,
	0,
	"123",
	"2.5",
	NULL,
	true,
	false,
	new stdClass,
	varray[],
	-PHP_INT_MAX-1,
	(string)(-PHP_INT_MAX-1),
];

foreach ($a as $var) {
	$var--;
	var_dump($var);
}

echo "Done\n";
}
