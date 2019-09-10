<?hh
<<__EntryPoint>> function main(): void {
  $a = array();
  $x = array_shift(&$a);
  var_dump($x);
  unset($a, $x);
  $a = array();
  $x = array_pop(&$a);
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
