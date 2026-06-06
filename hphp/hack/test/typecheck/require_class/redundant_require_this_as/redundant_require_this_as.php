<?hh
// (c) Meta Platforms, Inc. and affiliates.

<<file: __EnableUnstableFeatures('require_constraint')>>

// Test cases for redundant require this as warning

////  Scenario 1-BAD

// This SHOULD be flagged - trait has "require this as" but class doesn't use it
trait RRTAViolationTrait {
  require this as RRTAClassWithoutTrait;
}

class RRTAClassWithoutTrait {
}

final class RRTAClassWithoutTraitChild extends RRTAClassWithoutTrait {}

////  Scenario 1-OK

// This should NOT be flagged - trait has "require this as" and class DOES use it
trait RRTAValidTrait {
  require this as RRTAClassWithTrait;
}

class RRTAClassWithTrait {
  use RRTAValidTrait;
}

final class RRTAClassWithTraitChild extends RRTAClassWithTrait {}

////  Scenario 2

// This should NOT be flagged - trait has "require this as" and class DOES use it via another trait
trait RRTAValidTraitUse {
  require this as RRTAClassWithTraitUse;
}

trait RRTATraitWithUse {
  use RRTAValidTraitUse;
}

class RRTAClassWithTraitUse {
  use RRTATraitWithUse;
}

final class RRTAClassWithTraitUseChild extends RRTAClassWithTraitUse {}

//// Scenario 3-BAD

// This SHOULD be flagged - class does NOT use RRTATransitiveTrait
trait RRTATransitiveTrait {
  require this as RRTAClassTransitiveBad;
}

trait RRTAMiddleTrait {
  use RRTATransitiveTrait;
}

class RRTAClassTransitiveBad {
}

final class RRTAClassTransitiveBadChild extends RRTAClassTransitiveBad {}

/// Scenario 3-GOOD

// This should NOT be flagged - class USES RRTATransitiveTrait via RRTAMiddleTrait
trait RRTATransitiveTraitGood {
  require this as RRTAClassTransitiveGood;
}

trait RRTAMiddleTraitGood {
  use RRTATransitiveTraitGood;
}

class RRTAClassTransitiveGood {
  use RRTAMiddleTraitGood;
}

final class RRTAClassTransitiveGoodChild extends RRTAClassTransitiveGood {}

/// Scenario 4-SANITY

// This should NOT be flagged - uses require extends instead
trait RRTAExtendsTrait {
  require extends RRTAClassExtends;
}

class RRTAClassExtends {
}

final class RRTAClassExtendsChild extends RRTAClassExtends {}
