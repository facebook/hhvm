<?hh

<<__Memoize(#KeyedByIC)>>
function memo($a, $b)[zoned] :mixed{
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  echo "args: $a, $b hash:\n";
  echo HH\Lib\Str\join($hash, ", "); // we do Str\join here as zoned context won't allow var_dump
  echo "\n";
}

function g()[zoned] :mixed{
  memo(1, 2);
  memo(1, 3);
}

function f()[zoned] :mixed{
  memo(1, 2);
  memo(1, 3);
  ClassContext2::start(new B(0), g<>);
  memo(1, 2);
  memo(1, 3);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A(0), f<>);
}
