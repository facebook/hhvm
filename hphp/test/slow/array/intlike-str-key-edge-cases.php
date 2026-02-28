<?hh

<<__EntryPoint>>
function main_intlike_str_key_edge_cases() :mixed{
$i = PHP_INT_MAX;
$arr = dict[];
$arr[(string)$i] = 1;
$i = -$i;
$arr[(string)$i] = 1;
--$i;
$arr[(string)$i] = 1;
foreach ($arr as $k => $v) {
  var_dump($k);
}
}
