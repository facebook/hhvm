<?hh

<<__EntryPoint>>
function main_intlike_str_key_edge_cases() {
$i = PHP_INT_MAX;
$arr = darray[];
$arr[(string)$i] = 1;
$i = -$i;
$arr[(string)$i] = 1;
--$i;
$arr[(string)$i] = 1;
foreach ($arr as $k => $v) {
  var_dump($k);
}
}
