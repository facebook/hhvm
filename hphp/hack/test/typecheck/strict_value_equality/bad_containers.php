<?hh // strict

// These shouldn't typecheck
function bad_equality(
  vec<num> $var_vec_num,
  vec<int> $var_vec_int,
  vec<string> $var_vec_string,
  vec<dynamic> $var_vec_dynamic,
  dict<int, string> $var_dict_int_string,
  dict<int, int> $var_dict_int_int,
): void {
  $var_vec_num != $var_vec_int;
  $var_vec_int == $var_vec_string;
  $var_vec_num != $var_vec_dynamic;
  $var_dict_int_string == $var_dict_int_int;
}
