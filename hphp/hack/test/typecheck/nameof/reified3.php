<?hh

function expect_string(string $s): void {}
class C {}
class R {
  public function m<reify Tm as C>(): void {
    expect_string(Tm::class);
    expect_string(nameof Tm);
  }
}
