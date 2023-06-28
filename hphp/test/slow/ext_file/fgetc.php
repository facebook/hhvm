<?hh


<<__EntryPoint>>
function main_fgetc() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fgetc");
fclose($f);

$f = fopen($tempfile, "r");
var_dump(fgetc($f));
var_dump(fgetc($f));
var_dump(fgetc($f));
var_dump(fgetc($f));

unlink($tempfile);
}
