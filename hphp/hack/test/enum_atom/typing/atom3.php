<?hh
<<file:__EnableUnstableFeatures('enum_atom', 'enum_class')>>

class Box<T> {
  public function __construct(public T $x) {}
}

enum class Foo: mixed {
  Str<Box<string>>(new Box('zuck'));
}

class UseFoo {
  public function input<T>(
    <<__Atom>> HH\EnumMember<Foo, Box<T>> $atom, T $x
  ): void {}
  public function output<T>(<<__Atom>> HH\EnumMember<Foo, Box<T>> $atom): T {
    return $atom->data()->x;
  }
}

function test(UseFoo $foo): int {
  $foo->input(#Str, 1); // Should Error Here
  return $foo->output(#Str); // Should Error Here
}
