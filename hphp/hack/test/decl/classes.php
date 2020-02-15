<?hh

class MyClass {
  private static string $staticProperty;
  public string $instanceProperty;
  protected int $protectedProperty;
  <<__LateInit>> public arraykey $p;

  public function __construct(): void {}

  private function privateMethod(): void {}
  public function publicMethod(): void {}
  protected function protectedMethod(): void {}
  public async function async_generator(
    string $arg1,
    int $arg2,
  ): HH\AsyncGenerator<int, string, void> {
    await HH\Asio\usleep(500000);
    yield 0 => "test";
  }

  <<__Rx>>
  public function reactive_function(): void {}

  <<__RxShallow>>
  public function shallow_reactive_function(): void {}

  <<__RxLocal>>
  public function local_reactive_function(): void {}
}

abstract class MyAbstractClass {}
final class MyFinalClass {}
abstract final class MyStaticClass {}
