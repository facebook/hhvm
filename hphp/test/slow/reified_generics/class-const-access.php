<?hh

class C1 {
  const int A = 1;
}

class C2 {
  const int A = 2;
}

class D<reified T1> {
  public function f<reified T2>() {
    var_dump(T1::A);
    var_dump(T2::A);
  }
}

$d = new D<reified C1>();
$d->f<reified C2>();
