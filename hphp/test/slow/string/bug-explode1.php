<?hh


<<__EntryPoint>>
function main_bug_explode1() {
$c = str_repeat('*', 769000);
var_dump(explode($c, $c, PHP_INT_MIN));
}
