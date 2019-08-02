<?hh

class C {
  public static function f(): void {}
}

class Test<reify T as C> {
  public static function x(): void {
    T::f();
  }
}
