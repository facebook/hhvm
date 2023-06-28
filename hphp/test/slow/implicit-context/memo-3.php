<?hh

<<__Memoize(#KeyedByIC)>>
function memo()[zoned] :mixed{
  $hash = quoted_printable_encode(
    HH\ImplicitContext\_Private\get_implicit_context_memo_key()
  );
  echo "hash: $hash\n";
}

function g()[zoned] :mixed{
  memo();
  memo();
}

function f()[zoned] :mixed{
  memo();
  memo();
  ClassContext2::start(new B, g<>);
  memo();
  memo();
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
