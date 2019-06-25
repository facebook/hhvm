<?hh <<__EntryPoint>> function main(): void {
error_reporting(0);
$arr  = array("test");
try {
  list($a,$b) = is_array($arr)? $arr : $arr;
} catch (Exception $e) { echo $e->getMessage()."\n"; }
try {
  list($c,$d) = is_array($arr)?: NULL;
} catch (Exception $e) { echo $e->getMessage()."\n"; }

echo "ok\n";
}
