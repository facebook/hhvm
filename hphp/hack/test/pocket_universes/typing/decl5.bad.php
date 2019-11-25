<?hh // strict
// KO: reject dynamic/instance calls in definition of PU
class PU12 {
  public function stupid<T>(T $x): T { return $x; }

  enum X {
    case type T;
    case int v;
    :@X (type T = int, v = static::stupid<int>(1));
    :@Y (type T = int, v = unk->stupid<int>(1));
    :@Y (type T = int, v = this->stupid<int>(1));
  }
}
