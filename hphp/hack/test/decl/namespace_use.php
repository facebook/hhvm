<?hh //strict

namespace {
  namespace NS1 {
    namespace NS2 {
      trait MyTrait {}
    }
  }

  use namespace NS1\NS2;

  final class C {
    use NS2\MyTrait;
  }

}
