<?hh // partial

class C<T> {
  public function __construct(T $_) {}
}

function test(): void {
  $c = new C(42);
  if ($c is C<string>) {
    expect_Cstring($c);
  } else {
    hh_show($c);
  }
}

function expect_Cstring(C<string> $_): void {}
