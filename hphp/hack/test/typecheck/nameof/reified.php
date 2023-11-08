<?hh

<<file: __EnableUnstableFeatures('nameof_class')>>
function expect_string(string $s): void {}
class C {}
function f<reify T as C>(): void {
  expect_string(T::class);
  expect_string(nameof T);
}
class R<reify Tr as C> {
  public function m<reify Tm as C>(): void {
    expect_string(Tr::class);
    expect_string(nameof Tr);

    expect_string(Tm::class);
    expect_string(nameof Tm);
  }
}
