<?hh


<<__EntryPoint>>
function main_touch() {
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

touch($tempfile);
var_dump(file_exists($tempfile));

$tempfile2 = tempnam(sys_get_temp_dir(), 'vmextfiletest');

var_dump(file_exists($tempfile2));
copy($tempfile, $tempfile2);
var_dump(file_exists($tempfile2));

$tempfile3 = tempnam(sys_get_temp_dir(), 'vmextfiletest');

var_dump(file_exists($tempfile2));
rename($tempfile2, $tempfile3);
var_dump(file_exists($tempfile2));

var_dump(file_exists($tempfile3));

unlink($tempfile);
unlink($tempfile2);
unlink($tempfile3);
}
