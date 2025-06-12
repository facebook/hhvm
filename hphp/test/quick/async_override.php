<?hh

class Foo {
  public async function bar(int $value): Awaitable<int> {
    return $value * 2;
  }
}

class Bar extends Foo {
  public function bar(int $value): Awaitable<string> {
    return async {
      return "Processed: " . $value;
    };
  }
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  $obj = new Bar();
  var_dump(await $obj->doSomething(42));
}
