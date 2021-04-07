<?hh

abstract class B {
  abstract const ctx C super [defaults];
}

abstract class CAbstract extends B {
  // OK: moves the lower bound up toward pure ([])
  abstract const ctx C super [rx];
}

class BadConcreteViolatingExplicitBound extends CAbstract {
  // ERROR: [defaults] is not a supertype of [rx]
  const ctx C super [rx] = [defaults];
}
class BadConcreteViolatingInheritedBound extends CAbstract {
  // ERROR: [defaults] is not a supertype of [rx]
  const ctx C = [defaults];
}

class GoodConcreteWellRefinedPure extends CAbstract {
  // OK: further moves the lower bound up from [rx] to []
  const ctx C super [] = [];
}
class BadConcreteWronglyRefined extends CAbstract {
  // ERROR: C was refined to be >: [rx]
  const ctx C super [defaults] = [];
}
class GoodConcreteRefinedNoop extends CAbstract {
  const ctx C super [rx] = [];
}
