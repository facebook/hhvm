<?hh

enum E : string as string {
  A = "A";
  B = "B";
}

class C {
  public static function getE(): E {
    return E::A;
  }
}

function test(vec<int> $_):int {
  $e = C::getE();
  switch ($e) {
    case E::A : return 1;
    case E::B : return 2;
  }
}
