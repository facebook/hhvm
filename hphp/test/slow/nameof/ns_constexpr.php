<?hh

namespace N {
  enum E: string {
    A = nameof E;
    B = nameof \E;
  }
}
namespace {
  enum E: string {
    A = nameof E;
    B = nameof N\E;
  }

  <<__EntryPoint>>
  function main(): void {
    var_dump(N\E::A);
    var_dump(N\E::B);
    var_dump(E::A);
    var_dump(E::B);
  }
}
