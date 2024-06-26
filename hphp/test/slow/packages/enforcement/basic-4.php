<?hh

module z.z; // outside the active deployment

class FooBar extends Foo {
  public static function accessOtherModule() {
    Foo::foo();
  }
}

<<__EntryPoint>>
function main_4(): void {
  FooBar::accessOtherModule();
}
