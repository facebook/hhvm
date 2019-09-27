<?hh
<<__EntryPoint>> function main(): void {
  $a = array();
  $x = array_shift(inout $a);
  var_dump($x);
  unset($a, $x);
  $a = array();
  $x = array_pop(inout $a);
  var_dump($x);
  unset($a, $x);
  $a = array();
  $x = next(inout $a);
  var_dump($x);
  unset($a, $x);
  $a = array();
  $x = prev(inout $a);
  var_dump($x);
  unset($a, $x);
}
