<?hh

<<__PolicyShardedMemoize>>
function memo()[zoned] {
  $hash = quoted_printable_encode(
    HH\ImplicitContext\_Private\get_implicit_context_memo_key()
  );
  echo "hash: $hash\n";
}

function g()[zoned] {
  memo();
  memo();
}

function f()[zoned] {
  memo();
  memo();
  ClassContext2::start(new B, g<>);
  memo();
  memo();
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
