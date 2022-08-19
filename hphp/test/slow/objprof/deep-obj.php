<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function generate($arr) {
  for ($i = 0; $i < 2000; $i++) {
    $arr = vec[$arr];
  }
  return $arr;
}

class C {
  public vec $m_arr;
}

<<__EntryPoint>>
function main() {
  $arr = generate(vec[null]);
  $obj = new C;
  $obj->m_arr = $arr;
  var_dump(objprof_get_data());
  __hhvm_intrinsics\launder_value($obj);
}
