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
  $x = HH\class_meth($c, $f); var_dump(shuffle(&$x));
  $x = HH\class_meth($c, $f); var_dump(key(&$x));
  $x = HH\class_meth($c, $f); var_dump(reset(&$x));
  $x = HH\class_meth($c, $f); var_dump(each(&$x));
  $x = HH\class_meth($c, $f); var_dump(current(&$x));
  $x = HH\class_meth($c, $f); var_dump(next(&$x));
  $x = HH\class_meth($c, $f); var_dump(pos(&$x));
  $x = HH\class_meth($c, $f); var_dump(prev(&$x));
  $x = HH\class_meth($c, $f); var_dump(end(&$x));

  var_dump(array_diff(HH\class_meth($c, $f), [$f]));
  var_dump(array_udiff(HH\class_meth($c, $f), [$f], $cmp));
  var_dump(array_diff_assoc(HH\class_meth($c, $f), [$f]));
  var_dump(array_udiff_assoc(HH\class_meth($c, $f), [$f], $cmp));
  var_dump(array_udiff_uassoc(HH\class_meth($c, $f), [$f], $cmp, $cmp));
  var_dump(array_diff_key(HH\class_meth($c, $f), [0]));
  var_dump(array_diff_ukey(HH\class_meth($c, $f), [0], $cmp));

  var_dump(array_intersect(HH\class_meth($c, $f), [$f]));
  var_dump(array_uintersect(HH\class_meth($c, $f), [$f], $cmp));
  var_dump(array_intersect_assoc(HH\class_meth($c, $f), [$c]));
  var_dump(array_intersect_uassoc(HH\class_meth($c, $f), [$c], $cmp));
  var_dump(array_uintersect_assoc(HH\class_meth($c, $f), [$c], $cmp));
  var_dump(array_uintersect_uassoc(HH\class_meth($c, $f), [$c], $cmp, $cmp));
  var_dump(array_intersect_key(HH\class_meth($c, $f), [0]));
  var_dump(array_intersect_ukey(HH\class_meth($c, $f), [0], $cmp));

  $x = HH\class_meth($c, $f); var_dump(sort(&$x));
  $x = HH\class_meth($c, $f); var_dump(rsort(&$x));
  $x = HH\class_meth($c, $f); var_dump(asort(&$x));
  $x = HH\class_meth($c, $f); var_dump(arsort(&$x));
  $x = HH\class_meth($c, $f); var_dump(ksort(&$x));
  $x = HH\class_meth($c, $f); var_dump(krsort(&$x));

  $x = HH\class_meth($c, $f); var_dump(natsort(&$x));
  $x = HH\class_meth($c, $f); var_dump(natcasesort(&$x));

  $x = HH\class_meth($c, $f); var_dump(usort(&$x, $cmp));
  $x = HH\class_meth($c, $f); var_dump(uasort(&$x, $cmp));
  $x = HH\class_meth($c, $f); var_dump(uksort(&$x, $cmp));
  $x = HH\class_meth($c, $f); var_dump(array_multisort(&$x));
}

function test_string_builtins($c, $f) {
  var_dump(join(HH\class_meth($c, $f), '::'));
}

<<__EntryPoint>>
function main() {
  test_warning(
    A::class,
    'func1',
    ($l, $r) ==> { return $l > $r ? -1 : ($l === $r ? 0 : 1); });
}
