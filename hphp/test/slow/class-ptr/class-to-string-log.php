<?hh

class C {
  public static function s(string $s): void {}
}

<<__EntryPoint>>
function f(): void {
  $c = C::class;

  C::s(
    $c
  ); // log line 13

  $c::s(
    $c
  ); // should log on 17
}
