<?hh

newtype Impossible = nothing;

class A {
  public Impossible $foo;
  public int $foo2;
}

<<__EntryPoint>>
function main() : mixed {
  $x = new A()->foo2;
  var_dump($x);
}
