<?hh

<<__EntryPoint>>
function main() :mixed{
  $foo = __hhvm_intrinsics\launder_value(null);
  var_dump($foo?->bar(1, 2, 3));
}
