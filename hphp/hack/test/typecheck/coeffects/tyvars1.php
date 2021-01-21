<?hh

namespace HH\Contexts {
  type io = \HH\Capabilities\IO;
  namespace Unsafe {
    type io = mixed;
  }
}

namespace {
  function f()[]: (function()[io, write_props]: void) {
    return ()[io] ==> print "Hi!\n";
  }

  function test()[io]: void {
    caller(f(), f());
  }

  function caller(
    (function()[_]: void) $f1,
    (function()[_]: void) $f2,
  )[ctx $f1, ctx $f2]: void {
    $f1();
    $f2();
  }
}
