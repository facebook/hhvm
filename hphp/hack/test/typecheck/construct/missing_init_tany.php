<?hh

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}

namespace Test {
  type TnullableInt = ?int;
  class C {
    public ?int $x; // no error for nullable uninitialized prop
    public TnullableInt $y;
    public \HH_FIXME\MISSING_TYPE_IN_HIERARCHY $z;
    public dynamic $d;
    public ~int $li; // error here
  }
}
