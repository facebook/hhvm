<?hh

function h() :mixed{
  echo ClassContext2::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  echo var_dump($hash) . "\n";
}

function g()[zoned] :mixed{
  echo ClassContext2::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  echo var_dump($hash) . "\n";
  ClassContext2::start(new C(0), h<>);
}

function f()[zoned] :mixed{
  echo ClassContext::getContext()->name() . "\n";
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  echo var_dump($hash) . "\n";
  ClassContext2::start(new B(0), g<>);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A(0), f<>);
}
