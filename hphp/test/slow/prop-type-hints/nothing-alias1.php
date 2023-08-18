<?hh

newtype Impossible = nothing;

class A {
  public static Impossible $foo;
  public static int $foo2;
}

<<__EntryPoint>>
function main() : mixed {
  var_dump(A::$foo2);
}
