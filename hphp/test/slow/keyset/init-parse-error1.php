<?hh

<<__EntryPoint>> function main(): void {
  $ks = keyset[1 => 1, 'abc' => 2, 3 => 'def', 'ghi' => 'jkl'];
  var_dump($ks);
}
