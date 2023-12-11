<?hh
<<__EntryPoint>> function main(): void {
$a = vec[
	vec[1,2,3],
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
	vec[],
	-PHP_INT_MAX-1,
	(string)(-PHP_INT_MAX-1),
];

foreach ($a as $var) {
	try {
		$var--;
		var_dump($var);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

echo "Done\n";
}
