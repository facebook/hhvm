<?hh // strict

class A<T> {}

// These should all have typing errors.
function bad_equality(
  vec<A<int>> $var_vec_a_int,
  vec<A<int>> $var_vec_a_int2,
  A<int> $var_a_int,
  A<int> $var_a_int2,
): void {
  $var_vec_a_int == $var_vec_a_int2;
  $var_a_int != $var_a_int2;
}
