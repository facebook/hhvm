<?hh


<<__EntryPoint>>
function main_1775() :mixed{
error_reporting(0);
$tempfile = tempnam(sys_get_temp_dir(), 'lock');
$fp = fopen($tempfile, 'w');
fclose($fp);
$fp = fopen($tempfile, 'r+');
$wouldblock = false;
var_dump(flock($fp, 0xf0, inout $wouldblock));
fclose($fp);
unlink($tempfile);
}
