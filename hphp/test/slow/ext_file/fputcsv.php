<?hh


<<__EntryPoint>>
function main_fputcsv() :mixed{
$tempfile = tempnam(sys_get_temp_dir(), 'vmextfiletest');

$fields = vec['apple', "\"banana\""];
$f = fopen($tempfile, 'w');
// #2511892: have to specify all fields due to a bug
fputcsv($f, $fields, ',', '"');
fclose($f);

$f = fopen($tempfile, 'r');
fpassthru($f);
fclose($f);

$f = fopen($tempfile, 'r');
$vals = fgetcsv($f, 0, ',', '"', '\\'); // #2511892
var_dump($vals);

unlink($tempfile);
}
