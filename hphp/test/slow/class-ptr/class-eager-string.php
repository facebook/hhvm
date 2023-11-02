<?hh

class Foo {}
class Bar {
  const FOO = Foo::class;
  public static function sm() :mixed{
    var_dump("sm");
  }
  public static int $sp = 10;
  public function m() :mixed{
    var_dump("m");
  }
}

<<__EntryPoint>>
function main() :mixed{
  $c = __hhvm_intrinsics\create_class_pointer("Bar");
  var_dump($c);
  var_dump(is_scalar($c));
  var_dump(HH\class_get_class_name($c));
  $v = vec[
    __hhvm_intrinsics\create_class_pointer("Bar"),
    __hhvm_intrinsics\create_class_pointer("Fizz")
  ]; // Fizz is not a class
  var_dump($v);
  var_dump($c::FOO);
  $c::sm();
  var_dump($c::$sp);
  $o = new $c;
  $o->m();
  $f = __hhvm_intrinsics\create_class_pointer("Fizz");
  $o = new $f;  // should error as Fizz is not a class
}
