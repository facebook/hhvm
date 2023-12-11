<?hh
<<__EntryPoint>> function main(): void {
$a = dict[1=>0, 2=>1, 4=>3, "a"=>"b", "c"=>"d"];

try { var_dump(array_search(1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(array_search(1,1));
var_dump(array_search("a",$a));
var_dump(array_search("0",$a, true));
var_dump(array_search("0",$a));
var_dump(array_search(0,$a));
var_dump(array_search(1,$a));
var_dump(array_search("d",$a, true));
var_dump(array_search("d",$a));
var_dump(array_search(-1,$a, true));

echo "Done\n";
}
