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
function main_reflection_property_get_attribute() :mixed{
$rp = new ReflectionProperty(C::class, 'p');
var_dump($rp->getAttribute('B'));
var_dump($rp->hasAttribute('B'));
var_dump($rp->getAttribute('lol'));
var_dump($rp->hasAttribute('lol'));
$rp = new ReflectionProperty(C::class, 'sp');
var_dump($rp->getAttribute('D'));
var_dump($rp->hasAttribute('D'));
var_dump($rp->getAttribute('wut'));
var_dump($rp->hasAttribute('wut'));
$rp = new ReflectionProperty(C::class, 'pp');
var_dump($rp->getAttribute('F'));
var_dump($rp->hasAttribute('F'));
var_dump($rp->getAttribute('ohai'));
var_dump($rp->hasAttribute('ohai'));
$c = new C(false);
$c->dynamic = 'lol';
var_dump((new ReflectionProperty($c, 'dynamic'))->getAttribute('wry'));
var_dump((new ReflectionProperty($c, 'dynamic'))->hasAttribute('wry'));
}
