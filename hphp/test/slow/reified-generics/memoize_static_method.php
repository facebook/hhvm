<?hh
class C {
  <<__Memoize>>
  public static function f<reify Ta, Tb, reify Tc>($x, $y, $z) :mixed{
    var_dump("hi");
  }
}
class D {
  <<__Memoize>>
  public static function g<reify Ta, Tb, reify Tc>() :mixed{
    var_dump("hi");
  }
}

trait T {
  <<__Memoize>>
  public static function h<reify Ta, Tb, reify Tc>(mixed $x) :mixed{
    var_dump("hi");
  }
}

class E {
  use T;
}
<<__EntryPoint>>
function main_entry(): void {

  echo "multiple argument\n";

  C::f<int, string, bool>(1,2,3); // print
  C::f<int, string, bool>(1,2,3); // nope
  C::f<string, string, bool>(1,2,3); // print
  C::f<string, string, bool>(1,2,3); // nope
  C::f<string, string, string>(1,2,3); // print
  C::f<int, string, bool>(1,2,3); // nope
  C::f<int, string, bool>(1,1,3); // print
  C::f<int, string, bool>(1,2,3); // nope


  echo "no argument\n";

  D::g<int, string, bool>(); // print
  D::g<int, string, bool>(); // nope
  D::g<string, string, bool>(); // print
  D::g<string, string, bool>(); // nope
  D::g<string, string, string>(); // print
  D::g<int, string, bool>(); // nope
  D::g<int, string, bool>(); // nope

  echo "traits\n";

  E::h<int, string, bool>(1); // print
  E::h<int, string, bool>(1); // nope
  E::h<string, string, bool>(1); // print
  E::h<string, string, bool>(1); // nope
  E::h<string, string, string>(1); // print
  E::h<int, string, bool>(1); // nope
  E::h<int, string, bool>(1); // nope
  E::h<int, string, bool>(2); // print
  E::h<int, string, bool>(1); // nope
}
