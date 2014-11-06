<?hh // strict

enum MyEnum : string {
  A = 'a';
  B = 'b';
}

function f(MyEnum $e): void {
  // Force to be Tvar/Tunresolved
  if (true) {
    $e = MyEnum::A;
  }

  switch ($e) {
  }
}
