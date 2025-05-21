<?hh

<<__Memoize(#KeyedByIC)>>
function memo()[zoned] :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  $str_hash = HH\Lib\Str\join($hash, ', '); // can't do var_dump due to keyedByIC
  echo "hash: $str_hash\n";
}

function g()[zoned] :mixed{
  memo();
  memo();
}

function f()[zoned] :mixed{
  memo();
  memo();
  ClassContext2::start(new B(0), g<>);
  memo();
  memo();
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A(0), f<>);
}
