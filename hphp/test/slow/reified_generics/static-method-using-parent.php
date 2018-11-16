<?hh

class A<reify T> {}

class C<reify Ta, reify Tb> extends A<reify int> {
  public static function f() {
    new parent();
  }
}
