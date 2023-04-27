<?hh

class C {
  public static function m(string $s): void {}
}

function main(): void {
  C::m("hello");
}
