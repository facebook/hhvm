<?hh
<<file: __EnableUnstableFeatures('enum_atom', 'enum_class')>>

interface I {}
class C implements I {
  public function __construct(public int $x) {}
}

enum class E: I {
  A<C>(new C(42));
}

// Error, needs to be refied
function fgen<TEnum as E>(<<__Atom>> HH\EnumMember<TEnum, C> $x): int {
  return $x->data()->x;
}

function fgen2<reify TEnum as E>(<<__Atom>> HH\EnumMember<TEnum, C> $x): int {
  return $x->data()->x;
}

abstract class Controller {
  abstract const type TEnum as E;
  public static function get(<<__Atom>> HH\EnumMember<this::TEnum, C> $x): int {
    return $x->data()->x;
  }
}

class XXX extends Controller {
  const type TEnum = E;
}

class YYY {
  public static function get(<<__Atom>> HH\EnumMember<XXX::TEnum, C> $x): int {
    return $x->data()->x;
  }
}

class ZZZ {
  // error, invalid call to abstract class constant
  public static function get(
    <<__Atom>> HH\EnumMember<Controller::TEnum, C> $x,
  ): int {
    return $x->data()->x;
  }
}
