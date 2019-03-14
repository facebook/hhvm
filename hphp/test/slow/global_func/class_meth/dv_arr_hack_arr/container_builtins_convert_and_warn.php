<?hh

class A {
  static public function func1() {
    return 1;
  }
}

/*
 * These builtins are compatible with arraylike type and will raise Notice.
 */
function test_compact_builtins($c, $f) {
  var_dump(HH\Lib\_Private\Native\first_key(HH\class_meth($c, $f)));
  var_dump(HH\Lib\_Private\Native\first(HH\class_meth($c, $f)));
  var_dump(HH\Lib\_Private\Native\last_key(HH\class_meth($c, $f)));
  var_dump(HH\Lib\_Private\Native\last(HH\class_meth($c, $f)));

  var_dump(array_keys(HH\class_meth($c, $f)));
  var_dump(array_values(HH\class_meth($c, $f)));

  var_dump(count(HH\class_meth($c, $f)));
  var_dump(sizeof(HH\class_meth($c, $f)));
  var_dump(array_count_values(HH\class_meth($c, $f)));

  var_dump(array_key_exists(1, HH\class_meth($c, $f)));
  var_dump(in_array($f, HH\class_meth($c, $f)));
  var_dump(array_search($f, HH\class_meth($c, $f)));
  var_dump(hphp_array_idx(HH\class_meth($c, $f), 0, null));

  var_dump(array_change_key_case(HH\class_meth($c, $f), CASE_UPPER));
  var_dump(array_fill_keys(HH\class_meth($c, $f), 'foo'));
  var_dump(array_unique(HH\class_meth($c, $f)));

  array_rand(HH\class_meth($c, $f), 1);
  var_dump(array_product(HH\class_meth($c, $f)));
  var_dump(array_sum(HH\class_meth($c, $f)));

  var_dump(array_chunk(HH\class_meth($c, $f), 1));
  var_dump(array_column(HH\class_meth($c, $f), 0));
  var_dump(array_slice(HH\class_meth($c, $f), 1));
  var_dump(array_combine(HH\class_meth($c, $f), vec[1,2]));
  var_dump(array_pad(HH\class_meth($c, $f), 3, 255));

  var_dump(array_flip(HH\class_meth($c, $f)));
  var_dump(array_reverse(HH\class_meth($c, $f)));
  // TODO(T41492579): implicit ClsMeth conversion doesn't work with LIters
  // var_dump(array_map(($n) ==> { return $n * $n; }, HH\class_meth($c, $f)));
  var_dump(array_merge(HH\class_meth($c, $f), vec[3,4]));
  var_dump(array_merge_recursive(HH\class_meth($c, $f), vec[3,4]));
  var_dump(array_replace(HH\class_meth($c, $f), vec[3,4]));
  var_dump(array_replace_recursive(HH\class_meth($c, $f), vec[3,4]));

  $x = HH\class_meth($c, $f); var_dump(array_pop(&$x)); var_dump($x);
  $x = HH\class_meth($c, $f); var_dump(array_push(&$x, "t1")); var_dump($x);
  $x = HH\class_meth($c, $f); var_dump(array_shift(&$x)); var_dump($x);
  $x = HH\class_meth($c, $f); var_dump(array_splice(&$x, 1)); var_dump($x);
  $x = HH\class_meth($c, $f); var_dump(array_unshift(&$x, "foo")); var_dump($x);
}

function test_string_builtins($c, $f) {
  var_dump(join(HH\class_meth($c, $f), '::'));
}

<<__EntryPoint>>
function main() {
  $c = A::class;
  $f = 'func1';
  test_compact_builtins($c, $f);

  test_string_builtins($c, $f);
}
