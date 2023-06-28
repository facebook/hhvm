<?hh

class A {
  const type T1 = ?A::T2::T3;
  const type T2 = A;
  const type T3 = int;
  function f() :mixed{
    return type_structure(static::class, 'T1');
  }
}

class B extends A {}
<<__EntryPoint>> function main(): void {
$b = new B();
var_dump($b->f());
}
