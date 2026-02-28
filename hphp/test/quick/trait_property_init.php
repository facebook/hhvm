<?hh

// Test situations where traits cause duplicate property names when
// preparing the array for 86pinit setup.

//////////////////////////////////////////////////////////////////////
// Do this thing with a non-scalar initializer

class C { const A = "ASD"; }
const MUST_PREPARE = "temp";

trait TNonScalar { private $nonScalarTraitProperty = C::A; }

abstract class RobustCreationInterface {}
abstract class Robustish extends RobustCreationInterface { use TNonScalar; }
abstract class Robust extends Robustish {}
abstract class RobustProfile extends Robust {}

trait WebScaleObjectTrait {
  use TNonScalar;
  protected $profilePicPrepared = MUST_PREPARE;
}

trait TRobustOGObject {
  use WebScaleObjectTrait;
}

trait TFBRobustOGObject {
  use TRobustOGObject;
}

abstract class RobustEnterpriseBusiness extends RobustProfile {
  use TFBRobustOGObject;
}

class RobustRegularEnterpriseBusiness extends RobustEnterpriseBusiness {}

//////////////////////////////////////////////////////////////////////
// Same case with a scalar initializer

trait TScalar { private $scalarTraitProperty = vec[]; }

abstract class RobustCreationInterface2 {}
abstract class Robustish2 extends RobustCreationInterface2 { use TScalar; }
abstract class Robust2 extends Robustish2 {}
abstract class RobustProfile2 extends Robust2 {}

trait WebScaleObjectTrait2 {
  use TScalar;
  protected $profilePicPrepared = MUST_PREPARE;
}

trait TRobustOGObject2 {
  use WebScaleObjectTrait2;
}

trait TFBRobustOGObject2 {
  use TRobustOGObject2;
}

abstract class RobustEnterpriseBusiness2 extends RobustProfile2 {
  use TFBRobustOGObject2;
}

class RobustRegularEnterpriseBusiness2 extends RobustEnterpriseBusiness2 {}

//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main() :mixed{
  $k = new RobustRegularEnterpriseBusiness;
  echo "ok\n";
  $k = new RobustRegularEnterpriseBusiness2;
  echo "ok\n";
}
