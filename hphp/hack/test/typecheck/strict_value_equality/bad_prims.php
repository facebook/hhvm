<?hh // strict

type ActuallyInt = int;

// These should all have typing errors.
function bad_equality(
  int $var_int,
  ActuallyInt $var_actually_int,
  float $var_float,
  num $var_num,
  bool $var_bool,
  null $var_null,
  string $var_string,
  arraykey $var_arraykey,
): void {
  $var_num == $var_int;
  $var_int == $var_float;
  $var_bool != $var_float;
  $var_null == $var_string;
  $var_int != $var_string;
  $var_actually_int != $var_float;
  $var_num == $var_arraykey;
}
