<?hh
class C {
  <<__Memoize>>
  public function f<reify Ta, Tb, reify Tc>($x, $y, $z) :mixed{
    var_dump("hi");
  }
}
class D {
  <<__Memoize>>
  public function g<reify Ta, Tb, reify Tc>() :mixed{
    var_dump("hi");
  }
}
<<__EntryPoint>>
function main_entry(): void {

  echo "multiple argument\n";

  $c = new C();
  $c->f<int, string, bool>(1,2,3); // print
  $c->f<int, string, bool>(1,2,3); // nope
  $c->f<string, string, bool>(1,2,3); // print
  $c->f<string, string, bool>(1,2,3); // nope
  $c->f<string, string, string>(1,2,3); // print
  $c->f<int, string, bool>(1,2,3); // nope
  $c->f<int, string, bool>(1,1,3); // print
  $c->f<int, string, bool>(1,2,3); // nope


  echo "no argument\n";

  $d = new D();
  $d->g<int, string, bool>(); // print
  $d->g<int, string, bool>(); // nope
  $d->g<string, string, bool>(); // print
  $d->g<string, string, bool>(); // nope
  $d->g<string, string, string>(); // print
  $d->g<int, string, bool>(); // nope
  $d->g<int, string, bool>(); // nope
}
