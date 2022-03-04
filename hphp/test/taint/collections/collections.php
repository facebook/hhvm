<?hh

function __source(): int { return 1; }
function __sink($input): void {}

function no_flow_to_sink(): void {
  $vec = vec[0, 0];
  __sink($vec[0]);
}

function source_through_vec_to_sink(): void {
  $vec = vec[];
  $vec[] = __source();
  __sink($vec[0]);
}

// A precise taint implementation would recognize this and not
// report this as a flow. For now, we report it.
function source_through_vec_stopped(): void {
  $vec = vec[];
  $vec[] = __source();
  $vec[] = 1;
  __sink($vec[1]);
}

function source_through_vec_at_index_to_sink(): void {
  $vec = vec[0, 0, 0];
  $vec[2] = __source();
  __sink($vec[2]);
}

// A precise taint implementation would recognize this and not
// report this as a flow. For now, we report it.
function source_through_vec_at_index_stopped(): void {
  $vec = vec[0, 0, 0];
  $vec[2] = __source();
  $vec[2] = 0;
  __sink($vec[2]);
}

function source_through_vec_copies_to_sink(): void {
  $foo = vec[1];
  $bar = $foo;
  $baz = $foo;
  $foo[] = __source();
  $bar[] = 1;
  $baz[] = __source();
  // Only foo and baz should be tainted.
  __sink($foo[1]);
  __sink($bar[1]);
  __sink($baz[1]);
}

function source_through_keyset_to_sink(): void {
  $keyset = keyset[];
  $keyset[] = __source();
  foreach ($keyset as $key) {
    __sink($key);
  }
}

function source_through_dict_tainted_key_to_sink(): void {
  $dict = dict[];
  $k = __source();
  $v = 1;
  $dict[$k] = $v;
  __sink($dict[1]);
}

function source_through_dict_tainted_value_to_sink(): void {
  $dict = dict[];
  $v = __source();
  $k = 1;
  $dict[$k] = $v;
  __sink($dict[1]);
}

function source_through_dict_iteration_to_sink(): void {
  $dict = dict[];
  $dict[__source()] = 1;
  foreach ($dict as $k => $v) {
    __sink($v);
  }
}

function source_through_vector_to_sink(): void {
  $vector = Vector {};
  $vector[] = __source();
  __sink($vector[0]);
}

function source_through_map_to_sink(): void {
  $map = Map {};
  $k = __source();
  $v = 1;
  $map[$k] = $v;
  __sink($map[1]);
}

// This does not work right now as we don't support builtins
function source_through_set_to_sink(): void {
  $set = Set {};
  $set[] = __source();
  __sink($set->firstKey());
}

function source_through_pair_to_sink(): void {
  $pair = Pair {__source(), 1};
  __sink($pair[1]);
}

function iterators_do_not_collide_within_function(): void {
  $keyset1 = keyset[];
  $keyset1[] = __source();
  $keyset2 = keyset[];
  $keyset2[] = 1;
  foreach ($keyset1 as $key) {
    __sink($key);
  }
  foreach ($keyset2 as $key) {
    __sink($key);
  }
}

function inner(): void {
  $untainted = keyset[];
  $untainted[] = 1;
  foreach ($untainted as $key) {
    __sink($key);
  }
}

function iterators_do_not_collide_across_calls() {
  $tainted = keyset[];
  $tainted[] = __source();
  foreach ($tainted as $key) {
    inner();
    __sink($key);
  }
}

<<__EntryPoint>> function main(): void {
  no_flow_to_sink();
  source_through_vec_to_sink();
  source_through_vec_stopped();
  source_through_vec_at_index_to_sink();
  source_through_vec_at_index_stopped();
  source_through_vec_copies_to_sink();
  source_through_keyset_to_sink();
  source_through_dict_tainted_key_to_sink();
  source_through_dict_tainted_value_to_sink();
  source_through_dict_iteration_to_sink();
  // Tests for older collections below. These are not as comprehensive
  // as the implementations are similar to the above - we just test
  // basic behavior and try to ensure things don't crash.
  source_through_vector_to_sink();
  source_through_map_to_sink();
  source_through_set_to_sink();
  source_through_pair_to_sink();
  iterators_do_not_collide_within_function();
  iterators_do_not_collide_across_calls();
}
