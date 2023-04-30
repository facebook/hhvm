<?hh


<<__EntryPoint>>
function main_fprintf() {
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$f = fopen($tempfile, 'w');
fprintf($f, "%s %s", "testing", "fprintf");
fclose($f);

$f = fopen($tempfile, "r");
fpassthru($f);
echo "\n";

unlink($tempfile);
}
