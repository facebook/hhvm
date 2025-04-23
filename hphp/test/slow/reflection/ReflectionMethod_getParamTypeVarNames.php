<?hh

class Foo<U> {
  final public function __construct(protected U $u) {$this->m = $u;}
  public function bar<T>(T $b) : T {return $b;}
  public function baz(U $c) : void {echo "class type var";}
  private U $m;
}

class ReifiedFoo<reify U as num> {
  public function bar<reify T>(T $b) : void {echo "method type var";}
  public function baz(U $c) : void {echo "class type var";}
  public function bah<T as num>() : T {return 2;}
  public function bleh() : void {echo "no type var";}
}

function foobar<reify T as arraykey>(T $key) {
  var_dump($key);
}

<<__EntryPoint>>
function main() : void {
  var_dump(new ReflectionClass('Foo')->getConstructor()->getParamTypeVarNames());
  var_dump(new ReflectionClass('Foo')->getMethod('bar')->getParamTypeVarNames());
  var_dump(new ReflectionClass('Foo')->getMethod('baz')->getParamTypeVarNames());
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('bar')->getParamTypeVarNames());
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('baz')->getParamTypeVarNames());
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('bah')->getParamTypeVarNames());
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('bleh')->getParamTypeVarNames());
  var_dump(new ReflectionFunction('foobar')->getParamTypeVarNames());
}
