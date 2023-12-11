<?hh
<<__EntryPoint>> function main(): void {
var_dump(error_get_last());
try { var_dump(error_get_last(true)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(error_get_last());

$a = dict['x' => 2];
$a->foo;

var_dump(error_get_last());

echo "Done\n";
}
