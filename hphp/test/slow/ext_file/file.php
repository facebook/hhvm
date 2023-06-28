<?hh


<<__EntryPoint>>
function main_file() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing\nfile\n");
fclose($f);

$items = file($tempfile);
var_dump($items);

unlink($tempfile);
}
