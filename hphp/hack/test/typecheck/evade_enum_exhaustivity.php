<?hh

enum E: string as string {
  A = "a";
  B = "b";
}

class C {
  public function getEnumWrong(): E { return E::A;}
}

function main(): void {
  $e = (new C())->getEnumWrong();
  switch ($e) {
    case E::A:
      break;
  }
}
