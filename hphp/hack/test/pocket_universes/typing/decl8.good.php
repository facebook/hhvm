<?hh // strict
// OK: v mapped to an int
class PU8 {
  enum X {
    case int v;
    :@X (v = 1);
  }
}
