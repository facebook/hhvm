<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

interface I {}
class Box implements I {
  public function __construct(public int $x)[] {}
}

enum class EE : I {
   Box A = new Box(42);
}

enum class FF : I extends EE {
   Box C = new Box(0);
}

function ff(<<__ViaLabel>> HH\MemberOf<EE, Box> $x) : int {
  return $x->x;
}

abstract class Controller {
  abstract const type TEnum as EE;

  public static function get(
    <<__ViaLabel>>HH\MemberOf<this::TEnum, Box> $x): int {
    return $x->x;
  }

  public static function getA() : int {
    return static::get(#A);
  }

  public static function getB() : int {
    return static::get(#B);
  }
}

class CEE extends Controller {
  const type TEnum = EE;
}

class CFF extends Controller {
  const type TEnum = FF;
}

<<__EntryPoint>>
function main(): void {
  ff(#A);
  ff(#C);
  CEE::get(#A);
  CEE::get(#C);
  CFF::get(#A);
  CFF::get(#C);
}
