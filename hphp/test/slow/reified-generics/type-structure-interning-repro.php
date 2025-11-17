<?hh

class A {
  const type T = int;
}

class B extends A {
  // The ordering forces this TS to be initialized as
  // [{"kind": Int64, "nullable": true}]
  const type Toops = ?self::T;
}

class C<reify T> {}
// By default, the ?int TS is interned as
// [{"nullable": true, "kind": Int64}].
class D extends C<?int> {}
class E extends C<B::Toops> {}

<<__EntryPoint>>
function main() {
  $d = new D();
  $e = new E();
}
