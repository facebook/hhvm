<?hh

<<__EntryPoint>>
function main(): void {
  echo "hello";
}

class Foo extends Bar {
  const int MY_CONST = 42;
  public string $name = "";

  public function greet(string $who): string {
    return "Hello, " . $who;
  }

  protected static async function helper(): Awaitable<void> {
    // body
  }
}

enum Color: string {
  RED = "red";
  BLUE = "blue";
}

type MyAlias = shape('x' => int, 'y' => string);

interface IFoo {
  public function doSomething(int $x): void;
}

trait MyTrait {
  require extends Foo;
  public function traitMethod(): void {
    // body
  }
}
