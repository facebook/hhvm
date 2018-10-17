<?hh

class A<reified T> {}

class C<reified Ta, reified Tb> extends A<reified int> {
  public static function f() {
    new parent();
  }
}
