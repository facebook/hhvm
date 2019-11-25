<?hh // strict
// KO: type error in v (simple types int vs string)
class PU10 {
  enum X {
    case int v;
    :@X (v = "hello");
  }
}
