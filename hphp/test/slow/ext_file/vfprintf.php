<?hh


<<__EntryPoint>>
function main_vfprintf() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
vfprintf($f, "%s %s", vec["testing", "vfprintf"]);
fclose($f);

$f = fopen($tempfile, "r");
fpassthru($f);
echo "\n";

unlink($tempfile);
}
