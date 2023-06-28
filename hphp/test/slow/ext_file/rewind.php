<?hh


<<__EntryPoint>>
function main_rewind() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, 'testing rewind');
fclose($f);

$f = fopen($tempfile, 'r');
var_dump(fread($f, 7));
var_dump(fread($f, 100));
var_dump(fread($f, 7));
rewind($f);
var_dump(fread($f, 7));
var_dump(fread($f, 100));

unlink($tempfile);
}
