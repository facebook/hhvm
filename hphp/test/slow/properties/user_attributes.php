<?hh

// testing that these all parse correctly; reflection is just a way to verify it
class C {
  <<A(1)>>
  public $a;
  <<B('two')>>
  public int $b;
  <<C(vec[3])>>
  public static $c;
  <<D(vec['four'])>>
  public static int $d;
}


<<__EntryPoint>>
function main_user_attributes() :mixed{
foreach ((new ReflectionClass(C::class))->getProperties() as $prop) {
  var_dump($prop->getAttributes());
}
}
