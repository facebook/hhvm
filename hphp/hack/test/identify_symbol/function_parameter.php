<?hh

class C {
  public function __construct(string $s) {}
}

function test(): void {
  $string = "aaaaaaaa";
  new C($string);
}
