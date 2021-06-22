<?hh
<<__EntryPoint>> function main(): void {
  $a = null;
  $e = keyset[];
  try {
    $a += $e;
  } catch (Exception $e) {
    var_dump($a);
  }
}
