<?hh

function takes_reified<reify T>(): void {}

final class Foo {
  const type TFilter = vec<self::TFilter>;

  public function demo(): void {
    takes_reified<self::TFilter>();
  }
}
