<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static function foo($a, $b): darray {
    $res = darray[];
    foreach ($a as $key => $value) {
      if (is_array($value)) {
        $new_value = self::foo($value, $b);
        if ($new_value) $res[$key] = $new_value;
      } else {
        $res[$key] = varray[$value];
      }
    }
    return $res;
  }
}


<<__EntryPoint>>
function main_hhbbc3() {
var_dump(
  A::foo(
    __hhvm_intrinsics\launder_value(['a', 'b', 'c']),
    __hhvm_intrinsics\launder_value([])
  )
);
}
