<?hh

class A {}

class C<reify T1, T2> {}

class D {
  const type T = int;
  function foo(): C<A, this::T> {
    return new C<A, this::T>();
  }
  static function foo_static(): C<A, this::T> {
    return new C<A, this::T>();
  }
  static function foo_self(): C<A, int> {
    return new C<A, self::T>();
  }

}

<<__EntryPoint>>
function main() :mixed{
  (new D)->foo();
  D::foo_self();
  D::foo_static();
  echo "done\n";
}
