<?hh

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}
namespace Test {
  type TnullableInt = ?int;

  class C {
    public static ?int $x; // no error for nullable uninitialized prop
    public static TnullableInt $y;
    public static \HH_FIXME\MISSING_TYPE_IN_HIERARCHY $z;
    public static dynamic $d;
    public static ~int $li; // error here
  }
}
