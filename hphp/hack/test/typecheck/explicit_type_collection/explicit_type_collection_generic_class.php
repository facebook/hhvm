<?hh // strict

class foo<T>{
  private T $var;
  public function __construct(T $x){ $this->var = $x; }
  public function f(T $value): vec<T> {
    return vec<T>[$value, $this->var];
  }
}
