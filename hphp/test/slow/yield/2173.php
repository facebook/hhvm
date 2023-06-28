<?hh

class A {
  public static function sgen() :AsyncGenerator<mixed,mixed,void>{
    $class = static::class;
    yield $class;
  }
  public function gen() :AsyncGenerator<mixed,mixed,void>{
    $class = static::class;
    yield $class;
  }
  public function foo() :mixed{
    return self::gen();
  }
}
class B extends A {
}
function t($x) :mixed{
 foreach ($x as $v) {
 var_dump($v);
 }
 }

<<__EntryPoint>>
function main_2173() :mixed{
t(B::sgen());
t(A::sgen());
$b = new B;
t($b->gen());
t($b->foo());
$a = new A;
t($a->gen());
t($a->foo());
}
