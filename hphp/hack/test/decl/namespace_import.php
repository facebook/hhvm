<?hh

namespace {
  namespace NS1 {
    namespace NS2 {
      type T = int;

      class C {}
    }
  }


  use NS1\NS2 as NS;

  function test(): NS\T {
    return 4;
  }
  <<__Sealed(NS\C::class)>>
  class D {}
}
