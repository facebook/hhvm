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
}

abstract class MyAbstractClass {}
final class MyFinalClass {}
abstract final class MyStaticClass {}

class MyConstructorPropertiesClass {
  public function __construct(
    private string $private,
    protected string $protected,
    public string $public,
    public string $hasDefault = "has default",
  ) {}
}
