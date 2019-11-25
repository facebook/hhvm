<?hh // strict
// OK: T mapped to a valid type
class PU4 {
  enum X {
    case type T;
    :@X (type T = int);
  }
}
