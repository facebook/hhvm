<?hh

<<__Memoize(#KeyedByIC)>>
function memo<reify T>()[zoned] :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  $str_hash = HH\Lib\Str\join($hash, ', '); // can't do var_dump due to keyedByIC
  $kind = HH\ReifiedGenerics\get_type_structure<T>()['kind'];
  echo "hash: $str_hash, kind: $kind\n";
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
  ClassContext2::start(new B(0), g<>);
  memo<int>();
  memo<int>();
  memo<string>();
  memo<string>();
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A(0), f<>);
}
