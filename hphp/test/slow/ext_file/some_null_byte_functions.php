<?hh


<<__EntryPoint>>
function main_some_null_byte_functions() :mixed{
$file = '/etc/passwd'.chr(0).'asdf';

var_dump(readlink($file));
var_dump(glob($file));
var_dump(rmdir($file));
var_dump(dir($file));
var_dump(opendir($file));
var_dump(tempnam(sys_get_temp_dir(), 'foo' . chr(0) . 'bar'));
}
