<?hh

<<__EntryPoint>>
function main() :mixed{
  $f1 = main<>;
  $f2 = __hhvm_intrinsics\launder_value($f1);

  $f1[0] .= 'x';
  $f2[0] .= 'y';

  var_dump($f1[0] .= 'a');
  var_dump($f2[0] .= 'b');

  var_dump($f1, $f2);
}
