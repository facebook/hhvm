<?hh

enum MyEnum: string {
  A = 'a';
  B = 'b';
}

function f(MyEnum $e): void {
  // Force to be Tvar/Tunion
  if (true) {
    $e = MyEnum::A;
  }

  switch ($e) {
    case MyEnum::A: break;
  }
}
