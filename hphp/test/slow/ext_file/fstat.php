<?hh


<<__EntryPoint>>
function main_fstat() {
$tempfile = tempnam('/tmp', 'vmextfiletest');

$f = fopen($tempfile, 'w');
fputs($f, "testing fseek");
fclose($f);

$f = fopen($tempfile, "r");
$stats = fstat($f);
var_dump($stats['size']);
var_dump(is_darray($stats));

unlink($tempfile);
}
