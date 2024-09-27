<?hh

namespace N {
  enum E: string {
    A = nameof E;
    B = nameof \E;
    C = nameof N\E;
  }
}
namespace {
  enum E: string {
    A = nameof E;
    B = nameof N\E;
  }
}
