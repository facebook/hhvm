<?hh

function h() :mixed{
  echo ClassContext2::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
}

function g()[zoned] :mixed{
  echo ClassContext2::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
  ClassContext2::start(new C, h<>);
}

function f()[zoned] :mixed{
  echo ClassContext::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
  ClassContext2::start(new B, g<>);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
