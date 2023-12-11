<?hh <<__EntryPoint>> function main(): void {
error_reporting(0);
$arr  = vec["test"];
try {
  list($a,$b) = is_array($arr)? $arr : $arr;
} catch (Exception $e) { echo $e->getMessage()."\n"; }


echo "ok\n";
}
