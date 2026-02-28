<?hh



<<__EntryPoint>>
function main_836() :mixed{
$x = Set {
1, 2, 3}
;
var_dump($x);
echo print_r($x, true);
debug_zval_dump($x);
echo var_export($x, true) . "\n";
echo json_encode($x) . "\n";
echo json_encode($x, JSON_PRETTY_PRINT) . "\n";
echo serialize($x) . "\n";
var_dump(unserialize(serialize($x)));
}
