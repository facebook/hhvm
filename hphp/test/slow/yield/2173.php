<?hh

class A {
  public static function sgen() {
    $class = static::class;
    yield $class;
  }
  public function gen() {
    $class = static::class;
    yield $class;
  }
  public function foo() {
    return self::gen();
  }
}
class B extends A {
}
function t($x) {
 foreach ($x as $v) {
 var_dump($v);
 }
 }

<<__EntryPoint>>
function main_2173() {
t(B::sgen());
t(A::sgen());
$b = new B;
t($b->gen());
t($b->foo());
$a = new A;
t($a->gen());
t($a->foo());
}
