<?hh


<<__EntryPoint>>
function main_count_recursive() {
$arr = [];
$arr[] = &$arr;
var_dump(count($arr, COUNT_RECURSIVE));
}
