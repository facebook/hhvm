<?hh

<<__PolicyShardedMemoize>>
function f()[zoned] {
  echo "ok\n";
}

<<__EntryPoint>>
function main() {
  f(); f(); f();
}
