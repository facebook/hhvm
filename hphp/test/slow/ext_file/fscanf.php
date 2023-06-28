<?hh


<<__EntryPoint>>
function main_fscanf() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fscanf");
fclose($f);

$f = fopen($tempfile, "r");
$res = fscanf($f, "%s %s");
var_dump($res);

unlink($tempfile);
}
