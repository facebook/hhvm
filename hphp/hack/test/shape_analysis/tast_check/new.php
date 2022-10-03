<?hh

class C {
  public function __construct(dict<string, mixed> $_): void {}
}

function f(): void {
  $d1 = dict[]; // This result is reported
  $d2 = dict[]; // This result is invalidated
  new C($d2);
}
