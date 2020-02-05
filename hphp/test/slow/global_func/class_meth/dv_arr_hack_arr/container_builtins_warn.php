<?hh

class A {
  static public function func1() {
    return 1;
  }
}

/*
 * These builtins are NOT compatible with arraylike and will raise Warning.
 */
function test_warning($c, $f, $cmp) {
  $x = HH\class_meth($c, $f); var_dump(shuffle(inout $x));
  $x = HH\class_meth($c, $f); var_dump(key($x));
  $x = HH\class_meth($c, $f); var_dump(reset(inout $x));
  $x = HH\class_meth($c, $f); var_dump(each(inout $x));
  $x = HH\class_meth($c, $f); var_dump(current($x));
  $x = HH\class_meth($c, $f); var_dump(next(inout $x));
  $x = HH\class_meth($c, $f); var_dump(prev(inout $x));
  $x = HH\class_meth($c, $f); var_dump(end(inout $x));

  var_dump(array_diff(HH\class_meth($c, $f), varray[$f]));
  var_dump(array_udiff(HH\class_meth($c, $f), varray[$f], $cmp));
  var_dump(array_diff_assoc(HH\class_meth($c, $f), varray[$f]));
  var_dump(array_diff_uassoc(HH\class_meth($c, $f), varray[$f], $cmp));
  var_dump(array_udiff_assoc(HH\class_meth($c, $f), varray[$f], $cmp));
  var_dump(array_udiff_uassoc(HH\class_meth($c, $f), varray[$f], $cmp, $cmp));
  var_dump(array_diff_key(HH\class_meth($c, $f), varray[0]));
  var_dump(array_diff_ukey(HH\class_meth($c, $f), varray[0], $cmp));

  var_dump(array_intersect(HH\class_meth($c, $f), varray[$f]));
  var_dump(array_uintersect(HH\class_meth($c, $f), varray[$f], $cmp));
  var_dump(array_intersect_assoc(HH\class_meth($c, $f), varray[$c]));
  var_dump(array_intersect_uassoc(HH\class_meth($c, $f), varray[$c], $cmp));
  var_dump(array_uintersect_assoc(HH\class_meth($c, $f), varray[$c], $cmp));
  var_dump(array_uintersect_uassoc(HH\class_meth($c, $f), varray[$c], $cmp, $cmp));
  var_dump(array_intersect_key(HH\class_meth($c, $f), varray[0]));
  var_dump(array_intersect_ukey(HH\class_meth($c, $f), varray[0], $cmp));

  $x = HH\class_meth($c, $f); var_dump(sort(inout $x));
  $x = HH\class_meth($c, $f); var_dump(rsort(inout $x));
  $x = HH\class_meth($c, $f); var_dump(asort(inout $x));
  $x = HH\class_meth($c, $f); var_dump(arsort(inout $x));
  $x = HH\class_meth($c, $f); var_dump(ksort(inout $x));
  $x = HH\class_meth($c, $f); var_dump(krsort(inout $x));

  $x = HH\class_meth($c, $f); var_dump(natsort(inout $x));
  $x = HH\class_meth($c, $f); var_dump(natcasesort(inout $x));

  $x = HH\class_meth($c, $f); var_dump(usort(inout $x, $cmp));
  $x = HH\class_meth($c, $f); var_dump(uasort(inout $x, $cmp));
  $x = HH\class_meth($c, $f); var_dump(uksort(inout $x, $cmp));
  $x = HH\class_meth($c, $f); var_dump(array_multisort1(inout $x));

  $x = HH\class_meth($c, $f); var_dump(count($x));
  $x = HH\class_meth($c, $f); var_dump(HH\is_list_like($x));
}

<<__EntryPoint>>
function main() {
  test_warning(
    A::class,
    'func1',
    ($l, $r) ==> { return $l > $r ? -1 : ($l === $r ? 0 : 1); });
}
