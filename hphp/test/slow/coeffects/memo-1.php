<?hh

<<__PolicyShardedMemoize>>
function f()[policied] {
  echo "ok\n";
}

<<__EntryPoint>>
function main() {
  f(); f(); f();
}
