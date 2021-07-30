<?hh

function h() {
  echo ClassContext2::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
}

function g() {
  echo ClassContext2::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
  ClassContext2::start(new C, h<>);
}

function f() {
  echo ClassContext::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo quoted_printable_encode($hash) . "\n";
  ClassContext2::start(new B, g<>);
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
