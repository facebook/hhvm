<?hh


<<__EntryPoint>>
function main_fwrite() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fwrite($f, "testing fwrite", 7);
fclose($f);

$f = fopen($tempfile, 'r');
fpassthru($f);
echo "\n";

unlink($tempfile);
}
