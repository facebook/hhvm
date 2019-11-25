<?hh // strict
// KO: T mapped to an invalid (unknown) type
class PU5 {
  enum X {
    case type T;
    :@X (type T = U);
  }
}
