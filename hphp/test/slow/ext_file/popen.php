<?hh


<<__EntryPoint>>
function main_popen() {
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, 'testing popen');
fclose($f);

var_dump(file_exists($tempfile));
var_dump(file_exists('somethingthatdoesntexist'));

$f = popen("cat $tempfile", 'r');
var_dump(fread($f, 20));
pclose($f);

$old_dir_path = getcwd();
$filename = str_replace(sys_get_temp_dir().'/', '', $tempfile);
chdir(sys_get_temp_dir());
$f = popen("cat $filename", 'r');
var_dump(fread($f, 20));
pclose($f);
chdir($old_dir_path);

unlink($tempfile);
}
