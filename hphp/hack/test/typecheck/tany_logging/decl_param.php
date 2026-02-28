<?hh

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}
namespace Test {
  type Tany = \HH_FIXME\MISSING_TYPE_IN_HIERARCHY;
  class C {
    public static function static_method_param(Tany $_, Tany $_): void {}

    public function method_param(Tany $_): void {}
  }

function fx_param(Tany $_): void {}

function inout_param(inout Tany $_): void {}

function variadic_param(Tany ...$_): void {}
}
