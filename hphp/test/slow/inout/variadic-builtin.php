<?hh

function main() {
  $x = [1, 2, 3];
  array_unshift(inout $x, 24);
  array_unshift(inout $x, 42, 'apple', 200);
  array_unshift(&$x, 8, 99, 'green');
  array_unshift(&$x, 16);
  var_dump($x);
  var_dump(array_shift(&$x));
  var_dump(array_shift(inout $x));
  var_dump(array_shift(&$x));
  var_dump(array_shift(inout $x));
  var_dump($x);
}

main();
