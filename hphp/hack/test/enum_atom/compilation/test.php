<?hh
<<file:__EnableUnstableFeatures('enum_atom', 'enum_class')>>

interface I {}

class Box<T> implements I {
  public function __construct(public T $data) {}
}

abstract final class HBox {
  public static function String() : Box<string> {
    return new Box('zuck');
  }
}

enum class E : I {
  A<Box<int>>(new Box(42));
  B<Box<string>>(HBox::String());
}

class BBox<T> extends Box<T> {
  public function read() : T { return $this->data; }
}

enum class F : I extends E {
  C<BBox<num>>(new BBox(3.14));
}

abstract class Controller {
  abstract const type TEnum as E;

  public static
    function get<T>(HH\EnumMember<this::TEnum, Box<T>> $enum) : T {
    return $enum->data()->data;
  }

  public static
    function show<T>(HH\EnumMember<this::TEnum, Box<T>> $enum) : void {
    echo static::get($enum);
    echo "\n";
  }

  public static
    function show_atom<T>(
      <<__Atom>>HH\EnumMember<this::TEnum, Box<T>> $enum
    ) : void {
    echo static::get($enum);
    echo "\n";
  }
}

final class CE extends Controller {
  const type TEnum = E;
}

final class CF extends Controller {
  const type TEnum = F;
}
<<__EntryPoint>>
function main() : void {
  CE::show(E::A);
  CE::show(E::B);
  CF::show(F::C);
  echo "\n";
  CE::show_atom(#A);
  CE::show_atom(#B);
  CF::show_atom(#C);
}
