<?hh
<<__EntryPoint>> function main(): void {
  $a = dict[];
  $x = array_shift(inout $a);
  var_dump($x);
  unset($a, $x);
  $a = dict[];
  $x = array_pop(inout $a);
  var_dump($x);
  unset($a, $x);
}
