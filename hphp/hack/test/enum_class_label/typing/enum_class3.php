<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

class Box<T> {
  public function __construct(public T $x)[] {}
}

enum class Foo: mixed {
   Box<string> Str = new Box('zuck');
}

class UseFoo {
  public function input<T>(
    <<__ViaLabel>> HH\MemberOf<Foo, Box<T>> $elt, T $x
  ): void {}
  public function output<T>(<<__ViaLabel>> HH\MemberOf<Foo, Box<T>> $elt): T {
    return $elt->x;
  }
}

function test(UseFoo $foo): int {
  $foo->input(#Str, 1); // Should Error Here
  return $foo->output(#Str); // Should Error Here
}
