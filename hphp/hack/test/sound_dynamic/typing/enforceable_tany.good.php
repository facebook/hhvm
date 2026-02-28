<?hh

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}
namespace Test {
  class D {
    const type TC = \HH_FIXME\MISSING_TYPE_IN_HIERARCHY;
  }

  <<__SupportDynamicType>>
  class C {
    public static \HH_FIXME\MISSING_TYPE_IN_HIERARCHY $x = 3;
    public static D::TC $y = "A";
  }
}
