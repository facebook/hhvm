<?hh

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}
namespace Test {
  type Tany = \HH_FIXME\MISSING_TYPE_IN_HIERARCHY;

  class C {
    public static function f(): Tany {}

    public function g(): Tany {}
  }

  function h(): Tany {}
}
