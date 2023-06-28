<?hh
// Generate giant string and store it into APC
function dump($i, $flag): int {

  $i__str = (string)($i);

  $keyi = (string)(ApcOom::$prefixKey) . "{$i__str}";
  $i__str = (string)($i);
  $valuei = (string)(ApcOom::$prefixValue) . "{$i__str}";
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


<<__EntryPoint>>
function main_apc_oom() :mixed{
$length = 10000000;
$prefixKey = str_repeat("a",$length);
$prefixValue = str_repeat("b",$length);

echo "APC out of memory test begins!\n";
OOM_auto_trigger_test();
OOM_leak_test();
echo "passed\n";
}

abstract final class ApcOom {
  public static $prefixKey;
  public static $prefixValue;
}
