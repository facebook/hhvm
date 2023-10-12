<?hh // strict

// These should typecheck fine.
function good_equality(
  vec<int> $var_vec_int,
  vec<int> $var_vec_int2,
  vec<string> $var_vec_string,
  vec<dynamic> $var_vec_dynamic,
  dict<int, string> $var_dict_int_string,
  dict<int, string> $var_dict_int_string2,
  vec<vec<int>> $var_nested_vec,
  vec<vec<int>> $var_nested_vec2,
): void {
  $var_vec_int == $var_vec_int2;
  $var_vec_string != $var_vec_dynamic;
  $var_dict_int_string2 == $var_dict_int_string;
  $var_nested_vec != $var_nested_vec2;
}
