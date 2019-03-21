<?hh

<<__Rx>>
function test(...$args) {
  return func_num_args();
}

<<__EntryPoint>>
function main() {
  test(1);
  echo "Done\n";
}
