<?hh

type write_props = \HH\Capabilities\WriteProperty;

enum class A:arraykey {
  int X = 1;
  int Y = B::Y;
  int Z = true  ? 3 : B::Y;
  int W = C::w();
}
enum class B:string {
  arraykey Y = 2;
}
class C {
  public static function w()[write_props]:int {return 1;}
}
