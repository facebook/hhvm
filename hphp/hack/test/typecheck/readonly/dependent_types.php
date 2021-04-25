<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Bar {
}
abstract class Foo {
  abstract const type T as Foo;
  public function __construct(
    public readonly this::T $foo,
    public Bar $bar,
    public readonly Bar $ro_bar
  ) {}
  public function foo(this::T $x) : void {
    // error $this->foo is readonly
    $this->foo->bar = readonly new Bar();
    // error bar is mutable prop
    $x->bar = readonly new Bar();
    // ok
    $x->ro_bar = readonly new Bar();
  }
}
class Something extends Foo {
  const type T = Foo;
}
