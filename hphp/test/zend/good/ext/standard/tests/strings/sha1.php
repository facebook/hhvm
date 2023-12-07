<?hh
<<__EntryPoint>> function main(): void {
$filename = sys_get_temp_dir().'/'.'sha1.dat';
$a = varray[
	"abc",
	"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
	"a",
	"0123456701234567012345670123456701234567012345670123456701234567",
	""
];

foreach ($a as $str) {
	var_dump($val1 = sha1($str));
	file_put_contents($filename, $str);
	var_dump($val2 = sha1_file($filename));
	var_dump($val1 === $val2);
}

var_dump(sha1($str, true));
var_dump(sha1_file($filename, true));

unlink($filename);

sha1_file($filename);

echo "Done\n";
}
