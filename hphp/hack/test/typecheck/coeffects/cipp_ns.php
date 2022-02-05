<?hh

namespace {
  class A {}
}

namespace User {
  class A {}
  function policied_user_a()[zoned_with<A>]: void {}
  function policied_root_a()[zoned_with<\A>]: void {}
  // test builtins don't get elaborated in general
  function policied_mixed()[zoned_with<mixed>]: void {}
  function policied_nothing()[zoned_with<nothing>]: void {}

  function err()[zoned]: void {
    policied_user_a();
    policied_root_a();
    policied_mixed();
    policied_nothing();
  }
}
