<?hh


<<__EntryPoint>>
function main_ftruncate() {
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing ftruncate");
fclose($f);

$f = fopen($tempfile, "r+");
ftruncate($f, 7);
fclose($f);

$f = fopen($tempfile, "r");
var_dump(fread($f, 20));

unlink($tempfile);
}
