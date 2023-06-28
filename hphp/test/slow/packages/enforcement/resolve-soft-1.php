<?hh

module a.b;

<<__EntryPoint>>
function main_resolve_soft_1() :mixed{
  soft_baz1();

  $f = soft_baz2();
  $f();

  $f = soft_baz3();
  $f();

  soft_baz4();

  $f = soft_baz5();
  $f();

  $c = soft_cbaz1();
  new $c;
  $c::static_foo();

  $c = soft_cbaz2();
  $f();

  soft_cbaz3();
}
