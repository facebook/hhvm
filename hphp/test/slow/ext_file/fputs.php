<?hh


<<__EntryPoint>>
function main_fputs() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fputs\n");
fclose($f);

$f = fopen($tempfile, "r");
fpassthru($f);

unlink($tempfile);
}
