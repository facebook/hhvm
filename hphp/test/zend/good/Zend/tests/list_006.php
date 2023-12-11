<?hh
<<__EntryPoint>> function main(): void {
try {
  list($a, list($b, list(list($d)))) = vec[];
} catch (Exception $e) { echo $e->getMessage()."\n"; }
}
