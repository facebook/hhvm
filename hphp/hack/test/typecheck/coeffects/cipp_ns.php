<?hh

namespace {
  class A {}
}

namespace User {
  class A {}
  function cipp_user_a()[cipp_of<A>]: void {}
  function cipp_root_a()[cipp_of<\A>]: void {}
  // test builtins don't get elaborated in general
  function cipp_mixed()[cipp_of<mixed>]: void {}
  function cipp_nothing()[cipp_of<nothing>]: void {}

  function err()[cipp_global]: void {
    cipp_user_a();
    cipp_root_a();
    cipp_mixed();
    cipp_nothing();
  }
}
