<?hh

function show(): void {
  echo "show()\n";
  gc_collect_cycles();
  // Normal gc will also free APC vlaues for now.
}
function dump($i): string {
  $keyi = "AAA"."{$i}";
  $nestArray = ["AAA.{$i}","BBBBBBBBBBBBBBB.{$i}"];
  $valuei = [$nestArray,"valuess"."{$i}"];
  apc_store($keyi,$valuei);
  $r = apc_fetch($keyi);
  gc_collect_cycles();
  apc_delete($keyi);
  show(); // If we free APC value here
  // $r will be not accessible and crashed
  return $r[0][0];
}
function apc_gc_prematurely_free_test(): void {
  $si = dump(0); // Create the 1st APC entry
  dump(1); // Create the 2nd APC entry
  // Should be able to free the 2nd APC entry right now.
  // But can't free the 1st APC entry because of $si
  echo $si; // Crash if the 1st APC entry have already been freed
  echo "\n";
}
apc_gc_prematurely_free_test();
