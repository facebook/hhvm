<?hh

<<__EntryPoint>>
function main_power_multiply_equivalence() :mixed{
  var_dump(3 ** 2);
  var_dump(3 ** 2.0);
  var_dump(3 ** 4);
  var_dump(PHP_INT_MAX ** 2);
  var_dump(PHP_INT_MIN ** 2);
  var_dump(acos(8.0) ** 2);
  var_dump(0 ** 2);

  var_dump(2 ** 3);
  var_dump(2 ** 3.0);
  $root = intval(sqrt(floatval(PHP_INT_MAX)));
  var_dump($root ** 3);
  var_dump(PHP_INT_MAX ** 3);
  var_dump(PHP_INT_MIN ** 3);
  var_dump(acos(8.0) ** 3);
  var_dump(0 ** 3);
}
