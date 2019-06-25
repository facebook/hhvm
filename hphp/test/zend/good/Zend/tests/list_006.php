<?hh
<<__EntryPoint>> function main(): void {
try {
  list($a, list($b, list(list($d)))) = array();
} catch (Exception $e) { echo $e->getMessage()."\n"; }
}
