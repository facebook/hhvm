<?hh

namespace {
  class A {}
}

namespace User {
  class A {}
  function policied_user_a()[policied_of<A>]: void {}
  function policied_root_a()[policied_of<\A>]: void {}
  // test builtins don't get elaborated in general
  function policied_mixed()[policied_of<mixed>]: void {}
  function policied_nothing()[policied_of<nothing>]: void {}

  function err()[policied]: void {
    policied_user_a();
    policied_root_a();
    policied_mixed();
    policied_nothing();
  }
}
