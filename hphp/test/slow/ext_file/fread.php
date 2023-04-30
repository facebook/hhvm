<?hh


<<__EntryPoint>>
function main_fread() {
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fread");
fclose($f);

$f = fopen($tempfile, "r");
echo fread($f, 7);
echo fread($f, 100);
echo "\n";

unlink($tempfile);
}
