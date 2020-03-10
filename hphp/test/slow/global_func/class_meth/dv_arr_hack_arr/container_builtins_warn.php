<?hh

function CM($c, $m) { return __hhvm_intrinsics\create_clsmeth_pointer($c, $m); }

class A {
  static public function func1() { return 1; }
}

/*
 * These builtins are NOT compatible with arraylike and will raise Warning.
 */
function test_warning($c, $f, $cmp) {
  $x = CM($c, $f); var_dump(shuffle(inout $x));
  $x = CM($c, $f); var_dump(key($x));
  $x = CM($c, $f); var_dump(reset(inout $x));
  $x = CM($c, $f); var_dump(each(inout $x));
  $x = CM($c, $f); var_dump(current($x));
  $x = CM($c, $f); var_dump(next(inout $x));
  $x = CM($c, $f); var_dump(prev(inout $x));
  $x = CM($c, $f); var_dump(end(inout $x));

  var_dump(array_diff(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$f])));
  var_dump(array_udiff(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$f]), $cmp));
  var_dump(array_diff_assoc(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$f])));
  var_dump(array_diff_uassoc(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$f]), $cmp));
  var_dump(array_udiff_assoc(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$f]), $cmp));
  var_dump(array_udiff_uassoc(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$f]), $cmp, $cmp));
  var_dump(array_diff_key(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[0])));
  var_dump(array_diff_ukey(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[0]), $cmp));

  var_dump(array_intersect(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$f])));
  var_dump(array_uintersect(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$f]), $cmp));
  var_dump(array_intersect_assoc(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$c])));
  var_dump(array_intersect_uassoc(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$c]), $cmp));
  var_dump(array_uintersect_assoc(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$c]), $cmp));
  var_dump(array_uintersect_uassoc(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$c]), $cmp, $cmp));
  var_dump(array_intersect_key(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[0])));
  var_dump(array_intersect_ukey(CM($c, $f), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[0]), $cmp));

  $x = CM($c, $f); var_dump(sort(inout $x));
  $x = CM($c, $f); var_dump(rsort(inout $x));
  $x = CM($c, $f); var_dump(asort(inout $x));
  $x = CM($c, $f); var_dump(arsort(inout $x));
  $x = CM($c, $f); var_dump(ksort(inout $x));
  $x = CM($c, $f); var_dump(krsort(inout $x));

  $x = CM($c, $f); var_dump(natsort(inout $x));
  $x = CM($c, $f); var_dump(natcasesort(inout $x));

  $x = CM($c, $f); var_dump(usort(inout $x, $cmp));
  $x = CM($c, $f); var_dump(uasort(inout $x, $cmp));
  $x = CM($c, $f); var_dump(uksort(inout $x, $cmp));
  $x = CM($c, $f); var_dump(array_multisort1(inout $x));

  $x = CM($c, $f); var_dump(count($x));
  $x = CM($c, $f); var_dump(HH\is_list_like($x));
}

<<__EntryPoint>>
function main() {
  test_warning(
    A::class,
    'func1',
    ($l, $r) ==> { return $l > $r ? -1 : ($l === $r ? 0 : 1); });
}
