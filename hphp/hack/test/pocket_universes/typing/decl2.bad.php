<?hh // strict
// KO: v mapped to an incorrect expression
class PU9 {
  enum X {
    case int v;
    :@X (v = 1[1]);
  }
}
