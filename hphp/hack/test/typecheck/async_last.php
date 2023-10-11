<?hh

class Foo {
  // these will be fine
  public static async function bar(): Awaitable<void> {}
  public static function bar2(): void {}

  // this one will cause an error because async is not last
  public async static function baz(): Awaitable<void> {}
}
