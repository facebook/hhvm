<?hh

<<__Rx>>
function gen_safe() {
  yield 1;
  yield 2;
}

<<__Rx>>
function gen_bad() {
  yield from gen_safe();
  yield 3;
}

<<__EntryPoint>>
function test() {
  foreach (gen_bad() as $v) echo "$v\n";
}
