<?hh
function tuple($x) :mixed{
  echo "Inside tuple\n";
}
class C {
  public function tuple($x) :mixed{
    echo "Inside C::tuple\n";
  }
}
class D {
  public static function tuple($x) :mixed{
    echo "Inside D::tuple\n";
  }
}


<<__EntryPoint>>
function main_tuple_1() :mixed{
tuple(5);
(new C)->tuple(6);
D::tuple(7);
}
