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
  public function f() {
    $e = new E();
    $e->f<reified C::T2>();
  }
}

class E {
  const type T1 = bool;
  public function f<reified T>() {
    // we want T to be shape('a' => int, 'b' => int)
    //since self/this should be bound from class C
    var_dump(__hhvm_intrinsics\get_reified_type(T));
  }
}

$d = new D();

$d->f();
