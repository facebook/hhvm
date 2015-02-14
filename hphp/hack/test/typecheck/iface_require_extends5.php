<?hh // strict

interface IUseDynamicYield {}

trait DynamicYield implements IUseDynamicYield {}

trait THasProtectedFunc {
  require implements IUseDynamicYield;

  // These should be visible through C
  protected async function genFoo(): Awaitable<void> {}
  protected function bar(): void {}
}

abstract class C {
  use DynamicYield;
  use THasProtectedFunc;
}

interface IMarkerForC {
  require extends C;
}

trait TForMarkerForC {
  require implements IMarkerForC;

  protected async function test(): Awaitable<void> {
    await $this->genFoo();
    $this->bar();
  }
}
