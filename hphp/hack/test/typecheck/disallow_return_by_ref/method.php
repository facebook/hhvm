<?hh

class C {
  public function &test(): mixed {
    static $x;
    return $x;
  }
}
