<?hh
<<file: __EnableUnstableFeatures('type_splat', 'open_tuples')>>

abstract class C<T as (mixed...)> {
  abstract public function foo(
    ...T $args,
  ):void ;
  public function bar(
    ...T $args,
  ): void {
    var_dump($args);
  }
  public function baz(... T $args): void {
    echo "C::baz\n";
    var_dump($args);
  }
}
class D extends C<(int,string)> {
  public function foo(int $x, string $y): void {
    echo "D::foo\n";
    var_dump($x, $y);
  }
  public function bar(int $x, string $y): void {
    echo "D::bar\n";
    var_dump($x, $y);
  }
}

<<__EntryPoint>>
function main():void {
  $d = new D();
  $d->foo(3, "A");
  $d->bar(4, "B");
  $d->baz(5, "C");
}
