<?hh

class Foo<U> {
  final public function __construct(protected U $u) {$this->m = $u;}
  public function bar<T>(T $b) : T {return $b;}
  public function baz(U $c) : void {echo "class type var";}
  public function buzz<T>() : void {
    echo "generic method type var not present in fn header";
  }
  private U $m;
}

class ReifiedFoo<reify U as num> {
  public function bar<reify T>(T $b) : void {echo "method type var";}
  public function baz(U $c) : void {echo "class type var";}
  public function bah<T>() : T {return 2;}
  public function bleh() : void {echo "no type var";}
  public function meh<reify T>(vec<mixed> $vec) : void {
    foreach ($vec as $elem) {
      if ($elem is T) {
        echo "found ".T;
      }
    }
    echo "method type var used in body";
  }
  public function bzz<reify T, V, U>() : void {
    echo "method with mixed generics";
  }
}

function foobar<reify T as arraykey>(T $key) {
  var_dump($key);
}

<<__EntryPoint>>
function main() : void {
  var_dump("Foo::__construct");
  var_dump(new ReflectionClass('Foo')->getConstructor()->getTypeVarNames());
  var_dump("Foo::bar");
  var_dump(new ReflectionClass('Foo')->getMethod('bar')->getTypeVarNames());
  var_dump("Foo::baz");
  var_dump(new ReflectionClass('Foo')->getMethod('baz')->getTypeVarNames());
  var_dump("Foo::buzz");
  var_dump(new ReflectionClass('Foo')->getMethod('buzz')->getTypeVarNames());
  var_dump("ReifiedFoo::bar");
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('bar')->getTypeVarNames());
  var_dump("ReifiedFoo::baz");
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('baz')->getTypeVarNames());
  var_dump("ReifiedFoo::bah");
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('bah')->getTypeVarNames());
  var_dump("ReifiedFoo::bleh");
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('bleh')->getTypeVarNames());
  var_dump("ReifiedFoo::meh");
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('meh')->getTypeVarNames());
  var_dump("ReifiedFoo::bzz");
  var_dump(new ReflectionClass('ReifiedFoo')->getMethod('bzz')->getTypeVarNames());
  var_dump("foobar");
  var_dump(new ReflectionFunction('foobar')->getTypeVarNames());
  var_dump("Foo");
  var_dump(new ReflectionClass('Foo')->getTypeVarNames());
  var_dump("ReifiedFoo");
  var_dump(new ReflectionClass('ReifiedFoo')->getTypeVarNames());
}
