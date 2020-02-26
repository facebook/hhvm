<?hh
<<__EntryPoint>> function main(): void {
  $a = darray[];
  $x = array_shift(inout $a);
  var_dump($x);
  unset($a, $x);
  $a = darray[];
  $x = array_pop(inout $a);
  var_dump($x);
  unset($a, $x);
  $a = darray[];
  $x = next(inout $a);
  var_dump($x);
  unset($a, $x);
  $a = darray[];
  $x = prev(inout $a);
  var_dump($x);
  unset($a, $x);
}
