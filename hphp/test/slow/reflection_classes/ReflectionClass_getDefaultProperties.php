<?hh

class A {
  const X = 10;
  protected $foo;
  protected $bar = 1;
  protected $baz = self::X;
}

$class = new ReflectionClass(A::class);
var_dump($class->getDefaultProperties());

class B extends A {
  const Y = 20;
  protected $qux = A::X;
  protected $val = self::Y;
}

$class = new ReflectionClass(B::class);
var_dump($class->getDefaultProperties());

class C {
  protected $data = 1;
}

$class = new ReflectionClass(C::class);
var_dump($class->getDefaultProperties());
