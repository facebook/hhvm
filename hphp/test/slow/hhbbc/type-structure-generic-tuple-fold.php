<?hh

class A {
  const type T1 = (self::T2, A<B>, this::T2);
  const type T2 = A<B>;
  function f() :mixed{
    return type_structure(static::class, 'T1');
  }
}

class B extends A {}
<<__EntryPoint>> function main(): void {
$b = new B();
var_dump($b->f());
}
