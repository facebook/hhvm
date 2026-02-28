<?hh

class C {
  <<A, B(1, "two", vec[3])>>
  public $p;
  <<C, D('a', dict['b' => 'c'])>>
  public static $sp;
  public function __construct(
    <<E, F('promote', vec['me'])>>
    public $pp,
  ) {}
}

<<__EntryPoint>>
function main_reflection_property_get_attributes() :mixed{
var_dump((new ReflectionProperty(C::class, 'p'))->getAttributes());
var_dump((new ReflectionProperty(C::class, 'sp'))->getAttributes());
var_dump((new ReflectionProperty(C::class, 'pp'))->getAttributes());
$c = new C(false);
$c->dynamic = 'lol';
var_dump((new ReflectionProperty($c, 'dynamic'))->getAttributes());
}
