<?hh
type MyInt = int;
class B<reify T> {}
class A {
  public static function g(B<MyInt> $x): B<MyInt> {
    return $x;
  }
}

<<__EntryPoint>>
function f() :mixed{
  A::g(new B<int>());
  echo "done\n";
}
