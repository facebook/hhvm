<?hh

<<__Memoize(#KeyedByIC)>>
function memo($a, $b)[zoned] :mixed{
  $hash = quoted_printable_encode(
    HH\ImplicitContext\_Private\get_implicit_context_memo_key()
  );
  echo "args: $a, $b hash: $hash\n";
}

function g()[zoned] :mixed{
  memo(1, 2);
  memo(1, 3);
}

function f()[zoned] :mixed{
  memo(1, 2);
  memo(1, 3);
  ClassContext2::start(new B, g<>);
  memo(1, 2);
  memo(1, 3);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
