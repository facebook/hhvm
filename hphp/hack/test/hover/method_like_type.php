<?hh

class C {
  public function f(): void {}
}

class D {
  public static function make(C $x): C {
    return new C();
  }
  public function test(dynamic $x): void {
    self::make($x)->f();
//                  ^ hover-at-caret
  }
}
