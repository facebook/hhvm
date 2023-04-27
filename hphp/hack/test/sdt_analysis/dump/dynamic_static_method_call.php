<?hh

class C {
  public static function m(string $s): void {}
}

function main(dynamic $d): void {
  C::m($d);
}
