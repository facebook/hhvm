<?hh

interface IUseDynamicYield {}
trait DynamicYield implements IUseDynamicYield {
  public function __call(string $name, array $args = array()) {}
}

class Super {
  use DynamicYield;
}

interface ISuper extends IUseDynamicYield {}

trait TTestClass {
  require extends Super;

  public async function yieldXYZ(): Awaitable<void> {}

  public async function test(): Awaitable<void> {
    await $this->genXYZ();
  }
}

trait TTestInterface {
  require implements ISuper;

  public async function yieldXYZ(): Awaitable<void> {}

  public async function test(): Awaitable<void> {
    await $this->genXYZ();
  }
}
