<?hh

function expect_string(string $s): void {}
class C {}
class R<reify Tr as C> {
  public function m(): void {
    expect_string(Tr::class);
    expect_string(nameof Tr);
  }
}
