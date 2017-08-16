<?hh

$length = 10000000;
$prefixKey = str_repeat("a",$length);
$prefixValue = str_repeat("b",$length);
// Generate giant string and store it into APC
function dump($i, $flag): int {
  global $prefixKey, $prefixValue;
  $keyi = $prefixKey . "{$i}";
  $valuei = $prefixValue . "{$i}";
  apc_store($keyi,$valuei);
  if ($flag == true) gc_collect_cycles();
  apc_delete($keyi);
  return $i;
}
function OOM_auto_trigger_test(): void {
  $i = 1;
  $step = 2000;
  while($i < $step){
    $i++;
    dump($i,false);
  }
}
function OOM_leak_test(): void {
  $i = 1;
  $step = 2000;
  while($i < $step){
    $i++;
    dump($i+5000,true);
  }
}

echo "APC out of memory test begins!\n";
OOM_auto_trigger_test();
OOM_leak_test();
echo "passed\n";
