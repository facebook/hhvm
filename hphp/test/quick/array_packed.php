<?hh

function test($x) :mixed{
  // each of these should "just work" but stresses NewVArray/NewDArray logic.
  $a = vec[$x, 1, 2]; var_dump($a);
  $a = vec[$x,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]; var_dump($a);
  $a = vec[0,$x,0,0,0,0,0,8]; var_dump($a);
  $a = vec[0,$x,0,0,0,0,0,0,0,0,11]; var_dump($a);
  $a = vec[0,$x,0,0,0,0,0,0,0,0,0,12]; var_dump($a);
  $a = dict[
    0 => 1,
    1 => 1,
    2 => $x,
    "a" => $x
  ]; var_dump($a);
}
<<__EntryPoint>> function main(): void {
test(42);
}
