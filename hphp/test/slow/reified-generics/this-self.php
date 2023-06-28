<?hh

class C {
  const type T1 = int;
  const type T2 = shape(
    'a' => self::T1,
    'b' => this::T1,
  );
}

class D {
  const type T1 = string;
  public function f() :mixed{
    $e = new E();
    $e->f<C::T2>();
  }
}

class E {
  const type T1 = bool;
  public function f<reify T>() :mixed{
    // we want T to be shape('a' => int, 'b' => int)
    // since self/this should be bound from class C
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}
<<__EntryPoint>> function main(): void {
$d = new D();

$d->f();
}
