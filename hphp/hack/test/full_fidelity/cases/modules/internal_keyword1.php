<?hh
module foo;
internal abstract class Foo {
  internal function foo(): void {}
  internal abstract function bar(): void {}
  abstract internal function bar2(): void {}
  internal async function as(): Awaitable<void> {}
}

internal function foo(): void {}

internal trait TFoo {}

internal interface IFoo {}

<<SomeAttribute>>
internal class ClassWithAttribute {

}
