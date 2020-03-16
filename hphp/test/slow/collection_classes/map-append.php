<?hh

<<__EntryPoint>>
function main() {
  $m = Map {};
  $m[] = __hhvm_intrinsics\launder_value(Pair {1, 2});
  invariant($m->get(1) === 2, 'Still did the insert');
  $m[] = 42;
}
