<?hh

function CM($c, $m) { return __hhvm_intrinsics\create_clsmeth_pointer($c, $m); }

class A {
  static public function func1() { return 1; }
}

/*
 * These builtins are compatible with arraylike type and will raise Notice.
 */
function test_compact_builtins($c, $f) {
  var_dump(HH\Lib\_Private\Native\first_key(CM($c, $f)));
  var_dump(HH\Lib\_Private\Native\first(CM($c, $f)));
  var_dump(HH\Lib\_Private\Native\last_key(CM($c, $f)));
  var_dump(HH\Lib\_Private\Native\last(CM($c, $f)));

  var_dump(array_keys(CM($c, $f)));
  var_dump(array_values(CM($c, $f)));

  var_dump(count(CM($c, $f)));
  var_dump(sizeof(CM($c, $f)));
  var_dump(array_count_values(CM($c, $f)));

  var_dump(array_key_exists(1, CM($c, $f)));
  var_dump(in_array($f, CM($c, $f)));
  var_dump(array_search($f, CM($c, $f)));
  var_dump(hphp_array_idx(CM($c, $f), 0, null));

  var_dump(array_change_key_case(CM($c, $f), CASE_UPPER));

  var_dump(array_unique(CM($c, $f)));

  array_rand(CM($c, $f), 1);
  var_dump(array_product(CM($c, $f)));
  var_dump(array_sum(CM($c, $f)));

  var_dump(array_chunk(CM($c, $f), 1));
  var_dump(array_column(CM($c, $f), 0));
  var_dump(array_slice(CM($c, $f), 1));
  var_dump(array_combine(CM($c, $f), vec[1,2]));
  var_dump(array_pad(CM($c, $f), 3, 255));

  var_dump(array_flip(CM($c, $f)));
  var_dump(array_reverse(CM($c, $f)));


  var_dump(array_merge(CM($c, $f), vec[3,4]));
  var_dump(array_merge_recursive(CM($c, $f), vec[3,4]));
  var_dump(array_replace(CM($c, $f), vec[3,4]));
  var_dump(array_replace_recursive(CM($c, $f), vec[3,4]));

  $x = CM($c, $f); var_dump(array_pop(inout $x)); var_dump($x);
  $x = CM($c, $f); var_dump(array_push(inout $x, "t1")); var_dump($x);
  $x = CM($c, $f); var_dump(array_shift(inout $x)); var_dump($x);
  $x = CM($c, $f); var_dump(array_splice(inout $x, 1)); var_dump($x);
  $x = CM($c, $f); var_dump(array_unshift(inout $x, "foo")); var_dump($x);
}

function test_string_builtins($c, $f) {
  var_dump(join(CM($c, $f), '::'));
}

<<__EntryPoint>>
function main() {
  $c = A::class;
  $f = 'func1';
  test_compact_builtins($c, $f);

  test_string_builtins($c, $f);

  var_dump(array_map(($n) ==> { return $n * $n; }, CM($c, $f)));
}
