<?hh

interface Foo {}
interface DisposableFoo extends Foo, IDisposable {}
class FooImpl implements DisposableFoo {
  public function __dispose(): void {
  }
}

function inner(<<__AcceptDisposable>> Foo $_): void {
}

function mid(<<__AcceptDisposable>> DisposableFoo $a, Foo $b): void {
  inner($a);
  inner($b);
  using $c = new FooImpl();
  inner($c);
}
