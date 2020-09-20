<?hh

class A {
  public static function foo() : string {
    return "a";
  }
}

class B extends A {
  public static function foo() : string {
    return A::foo();
  }
}

class C extends A {
  public static function foo() : string {
    return "c";
  }
}
