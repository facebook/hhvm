<?hh

enum E: int as int {
  A = 1;
}
enum F: string as string {
  B = "B";
}
enum G: int {
  C = 2;
}

function f(
  enum<int> $e
): void {
  foreach ($e::getNames() as $n) {
    hh_expect_equivalent<string>($n);
  }
}

function g(): void {
  f(E::class);
  f(F::class);
  f(G::class);
  f(nameof E);
}
