<?hh

<<file: __EnableUnstableFeatures('case_types')>>

final class A {}
final class B {}
final class C {}

case type C1 = A | B;
case type C2 = B | C;

class T0<T as C1> {
  <<__LateInit>> public T $p0;
}

class T1<T as C1 as C2> {
  <<__LateInit>> public T $p0;
}

<<__EntryPoint>>
function main(): void {
  require "test.inc";
  throw_errors();

  $t = new T0();
  $t->p0 = new A();
  $t->p0 = new B();
  expect_exception(
    () ==> {
      $t->p0 = new C();
    },
    "Property 'T0::p0' upper-bounded by type C1, C assigned"
  );

  $t = new T1();
  expect_exception(
    () ==> {
      $t->p0 = new A();
    },
    "Property 'T1::p0' upper-bounded by type C2, A assigned"
  );
  $t->p0 = new B();
  expect_exception(
    () ==> {
      $t->p0 = new C();
    },
    "Property 'T1::p0' upper-bounded by type C1, C assigned"
  );

  echo "done\n";
}
