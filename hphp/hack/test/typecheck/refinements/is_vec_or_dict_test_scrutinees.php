<?hh

case type IntOrVecOrDictString = int | vec_or_dict<string>;
case type IntOrVecOrDictStringString = int | vec_or_dict<string, string>;
case type IntOrVecOrDictIntString = int | vec_or_dict<int, string>;
case type IntOrDict = int | dict<string, string>;
case type IntOrVec = int | vec<string>;
case type IntOrDictOrVec = int | dict<string, string> | vec<string>;

function test_scrutinee_vec_or_dict_string(IntOrVecOrDictString $x): int {
  if ($x is vec_or_dict<_>) {
    hh_show($x);
    return 0;
  } else {
    return $x;
  }
}

function test_scrutinee_vec_or_dict_string_string(
  IntOrVecOrDictStringString $x,
): int {
  if ($x is vec_or_dict<_>) {
    hh_show($x);
    return 0;
  } else {
    return $x;
  }
}

function test_scrutinee_vec_or_dict_int_string(
  IntOrVecOrDictIntString $x,
): int {
  if ($x is vec_or_dict<_>) {
    hh_show($x);
    return 0;
  } else {
    return $x;
  }
}

function test_scrutinee_dict(IntOrDict $x): int {
  if ($x is vec_or_dict<_>) {
    hh_expect_equivalent<dict<string, string>>($x);
    return 0;
  } else {
    return $x;
  }
}

function test_scrutinee_vec(IntOrVec $x): int {
  if ($x is vec_or_dict<_>) {
    hh_expect_equivalent<vec<string>>($x);
    return 0;
  } else {
    return $x;
  }
}

function test_scrutinee_dict_or_vec(IntOrDictOrVec $x): int {
  if ($x is vec_or_dict<_>) {
    hh_show($x);
    return 0;
  } else {
    return $x;
  }
}
