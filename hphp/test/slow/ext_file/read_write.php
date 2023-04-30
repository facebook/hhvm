<?hh


<<__EntryPoint>>
function main_read_write() {
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fwrite($f, "testing read/write", 7);
fclose($f);

$f = fopen($tempfile, "r+");
fseek($f, 8);
fwrite($f, "succeeds");
fseek($f, 8);
var_dump(fread($f, 8));

unlink($tempfile);
}
