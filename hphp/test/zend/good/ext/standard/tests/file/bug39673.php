<?hh
<<__EntryPoint>> function main(): void {
$str = str_repeat("test", 3456);

$filename = sys_get_temp_dir().'/'.'bug39673.txt';
file_put_contents($filename, $str);

$offsets = vec[
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

unlink($filename);
echo "Done\n";
}
