<?hh

class A{}

class C<reified Ta, reified Tb> {
  public static function f() {
    new A<reified Ta>();
  }
}
