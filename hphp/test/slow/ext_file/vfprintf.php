<?hh


<<__EntryPoint>>
function main_vfprintf() {
$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
vfprintf($f, "%s %s", varray["testing", "vfprintf"]);
fclose($f);

$f = fopen($tempfile, "r");
fpassthru($f);
echo "\n";

unlink($tempfile);
}
