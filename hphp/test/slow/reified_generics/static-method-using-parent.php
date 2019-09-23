<?hh

class A<reify T> {}

class C<reify Ta, reify Tb> extends A<int> {
  public static function f() {
    new parent();
  }
}

