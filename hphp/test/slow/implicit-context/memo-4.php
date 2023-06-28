<?hh

<<__Memoize(#KeyedByIC)>>
function memo<reify T>()[zoned] :mixed{
  $hash = quoted_printable_encode(
    HH\ImplicitContext\_Private\get_implicit_context_memo_key()
  );
  $kind = HH\ReifiedGenerics\get_type_structure<T>()['kind'];
  echo "hash: $hash, kind: $kind\n";
}

function g()[zoned] :mixed{
  memo<int>();
  memo<int>();
  memo<string>();
  memo<string>();
}

function f()[zoned] :mixed{
  memo<int>();
  memo<int>();
  memo<string>();
  memo<string>();
  ClassContext2::start(new B, g<>);
  memo<int>();
  memo<int>();
  memo<string>();
  memo<string>();
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
