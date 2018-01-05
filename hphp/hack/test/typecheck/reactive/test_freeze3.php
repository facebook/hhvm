<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

// freeze in multiple scopes
<<__Rx>>
function basic(): void {
  $z = new C(7); // $z is mutable
  freeze($z);
  // error, $z is already immutable
  freeze($z);
}
