<?hh


<<__EntryPoint>>
function main_link() {
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');
$tempfile2 = tempnam(sys_get_temp_dir(), 'vmextfiletest');

unlink($tempfile2);
var_dump(file_exists($tempfile2));
link($tempfile, $tempfile2);
var_dump(file_exists($tempfile2));

unlink($tempfile2);
var_dump(file_exists($tempfile2));
symlink($tempfile, $tempfile2);
var_dump(file_exists($tempfile2));

unlink($tempfile);
unlink($tempfile2);
}
