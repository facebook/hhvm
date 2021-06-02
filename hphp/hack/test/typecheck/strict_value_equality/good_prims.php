<?hh // strict

type ActuallyInt = int;

// These should typecheck fine.
function good_equality(
  int $var_int,
  float $var_float,
  ActuallyInt $var_actually_int,
  string $var_string1,
  string $var_string2,
  dynamic $var_dynamic,
): void {
  $var_int != 2;
  $var_float == 2.0;
  $var_actually_int == $var_int;
  $var_string1 == $var_string2;
  $var_dynamic == $var_string1;
}
