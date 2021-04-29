<?hh

abstract class B {
  abstract const ctx C super [defaults];
}

abstract class CAbstract extends B {
  // OK: moves the lower bound up toward pure ([])
  abstract const ctx C super [oldrx];
}

class BadConcreteViolatingExplicitBound extends CAbstract {
  // ERROR: [defaults] is not a supertype of [oldrx]
  const ctx C super [oldrx] = [defaults];
}
class BadConcreteViolatingInheritedBound extends CAbstract {
  // ERROR: [defaults] is not a supertype of [oldrx]
  const ctx C = [defaults];
}

abstract class GoodAbstractWellRefinedPure extends CAbstract {
  // OK: further moves the lower bound up from [oldrx] to []
  abstract const ctx C super [] = [];
}
abstract class BadAbstractWronglyRefined extends CAbstract {
  // ERROR: C was refined to be >: [oldrx]
  abstract const ctx C super [defaults];
}
abstract class GoodAbstractRefinedImpure extends CAbstract {
  abstract const ctx C super [oldrx];
}
