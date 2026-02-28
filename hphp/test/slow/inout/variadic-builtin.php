<?hh

function main() :mixed{
  $x = vec[1, 2, 3];
  array_unshift(inout $x, 24);
  array_unshift(inout $x, 42, 'apple', 200);
  array_unshift(inout $x, 8, 99, 'green');
  array_unshift(inout $x, 16);
  var_dump($x);
  var_dump(array_shift(inout $x));
  var_dump(array_shift(inout $x));
  var_dump(array_shift(inout $x));
  var_dump(array_shift(inout $x));
  var_dump($x);
}


<<__EntryPoint>>
function main_variadic_builtin() :mixed{
main();
}
