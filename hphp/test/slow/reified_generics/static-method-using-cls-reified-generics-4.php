<?hh

class C<reified Ta, reified Tb> {
  public static function f() {
    g<reified Ta>();
  }
}
