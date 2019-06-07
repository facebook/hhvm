<?hh


<<__EntryPoint>>
function main_return_null() {
$test=eval('return null;');
var_dump(gettype($test));
}
