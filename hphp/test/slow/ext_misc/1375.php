<?hh


<<__EntryPoint>>
function main_1375() {
$d=fopen(__DIR__.'/../../sample_dir/file', 'r');
var_dump(is_object($d));
var_dump(is_resource($d));
var_dump(gettype((string)$d));
}
