<?hh

class C1 {
  const int A = 1;
}

class C2 {
  const int A = 2;
}

class D<reify T1> {
  public function f<reify T2>() :mixed{
    var_dump(T1::A);
    var_dump(T2::A);
  }
}
<<__EntryPoint>> function main(): void {
$d = new D<C1>();
$d->f<C2>();
}
