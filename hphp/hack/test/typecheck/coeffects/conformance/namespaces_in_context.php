<?hh

namespace MyOuterNs {
  class Outer {}

  namespace MyInnerNs {
    class Inner {}

    function outer_from_inner_bad()[MyOuterNs\Outer]: void {}
  }

  function inner_from_outer_bad()[MyInnerNs\Inner]: void {}
}

namespace MyNamespace {
  function fully_qualified_bad()[\MyOuterNs\MyInnerNs\Inner]: void {}
}
