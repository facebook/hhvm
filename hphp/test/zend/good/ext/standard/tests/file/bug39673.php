<?hh
<<__EntryPoint>> function main(): void {
$str = str_repeat("test", 3456);

$filename = __SystemLib\hphp_test_tmppath('bug39673.txt');
file_put_contents($filename, $str);

$offsets = varray[
	-1,
	0,
	3456*4,
	3456*4 - 1,
	3456*4 + 1,
	2000,
	5000,
	100000,
];


foreach ($offsets as $offset) {
	$r = file_get_contents($filename, false, null, $offset);
	var_dump(strlen($r));
}

@unlink($filename);
echo "Done\n";
}
