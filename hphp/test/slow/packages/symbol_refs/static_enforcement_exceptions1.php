<?hh

module bar;

type A = shape(
  'bar' => string,
  'foo' => TFoo1,
);
type B = vec<TFoo1>;
<<CFoo()>> class C {
    const type TC = Foo;
}
type D = dict<string, TFoo1>;
type F1 = (function(TFoo1): int);
type F2 = (function(int): TFoo1);

interface I<T> {
  public function get(): T;
}
trait T implements I<?EFoo> {
  private ?EFoo $value;
  public function get(): ?EFoo {
    return $this->value;
  }
}
class TImpl implements I<?EFoo> {
    use T;
}

<<__EntryPoint>>
function main(): void {
    $_ = new C();
    $_ = shape('x' => 'hello', 'y' => 42);
    $_ = vec[42];
    $_ = dict['x' => 1, 'y' => 2];
    $_ = (TFoo1 $foo) ==> $foo as int;
    $_ = (int $value) ==> $value;
    $t = new TImpl();
    $_ = $t->get();
    echo "No errors\n";
  }
