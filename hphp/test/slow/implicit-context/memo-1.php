<?hh

<<__PolicyShardedMemoize>>
function memo($a, $b)[zoned] {
  $hash = quoted_printable_encode(
    HH\ImplicitContext\_Private\get_implicit_context_memo_key()
  );
  echo "args: $a, $b hash: $hash\n";
}

function g() {
  memo(1, 2);
  memo(1, 3);
}

function f() {
  memo(1, 2);
  memo(1, 3);
  ClassContext2::start(new B, g<>);
  memo(1, 2);
  memo(1, 3);
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
