<?hh

class A<T> {
  function __construct(public Vector<T> $f) {}
}
<<__EntryPoint>> function main(): void {
$a = new A<int>(Vector{1, 2, 3});
var_dump($a);
}
