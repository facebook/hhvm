<?hh // strict

<<__DynamicallyCallable>>
function foo(): void {}

<<__DynamicallyConstructible>>
interface I {
  <<__DynamicallyCallable>>
  public function bar(): void;
}

<<__DynamicallyConstructible>>
abstract class B implements I {
  <<__DynamicallyCallable>>
  public abstract static function genBaz(): Awaitable<void>;
}

<<__DynamicallyConstructible>>
class C extends B {
  <<__DynamicallyCallable>>
  final public function bar(): void {}

  <<__Override, __DynamicallyCallable>>
  public static async function genBaz(): Awaitable<void> {}
}
