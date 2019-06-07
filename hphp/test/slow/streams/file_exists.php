<?hh


<<__EntryPoint>>
function main_file_exists() {
$fnam = __FILE__;
var_dump(file_exists($fnam));
var_dump(file_exists('file://'.$fnam));
var_dump(file_exists('file://'.realpath($fnam)));
}
