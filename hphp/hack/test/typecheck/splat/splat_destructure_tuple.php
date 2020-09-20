<?hh

function f(int $i, string $s, bool $b = true, float $f = 3.14, int ...$is): void {}

function g(): void {
  f(...tuple(1)); // too few
  f(...tuple(1, "a"));
  f(...tuple(1, "a", false)); // partially filled optional parameters
  f(...tuple(1, "a", false, 2.0)); // fully filled optional parameters
  f(...tuple(1, "a", false, 2.0, 3, 4, 5));
  f(...tuple(1, "a", false, 2.0, 3, false, 5)); // incorrect type
}
