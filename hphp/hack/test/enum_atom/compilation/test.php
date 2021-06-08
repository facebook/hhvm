<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

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
   Box<int> A = new Box(42);
   Box<string> B = HBox::String();
}

class BBox<T> extends Box<T> {
  public function read() : T { return $this->data; }
}

enum class F : I extends E {
   BBox<num> C = new BBox(3.14);
}

abstract class Controller {
  abstract const type TEnum as E;

  public static
    function get<T>(HH\MemberOf<this::TEnum, Box<T>> $enum) : T {
    return $enum->data;
  }

  public static
    function show<T>(HH\MemberOf<this::TEnum, Box<T>> $enum) : void {
    echo static::get($enum);
    echo "\n";
  }

  public static
    function show_member_via_label<T>(
      <<__ViaLabel>>HH\MemberOf<this::TEnum, Box<T>> $enum
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
  CE::show_member_via_label(#A);
  CE::show_member_via_label(#B);
  CF::show_member_via_label(#C);
}
