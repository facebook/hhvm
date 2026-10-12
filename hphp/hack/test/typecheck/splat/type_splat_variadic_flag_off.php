<?hh

// With pure_variadic_splat_subtyping off, a lone variadic against a lone
// splat keeps the legacy element-wise comparison in both directions.

interface Splat<T as (string...)> {
  public function __construct(... T $values);
}

final class Impl implements Splat<(string...)> {
  public function __construct(string ...$values)[] {}
}

interface SplatArr<T as (arraykey)> {
  public function f(...T $values): void;
}

final class ImplArr implements SplatArr<(arraykey)> {
  public function f((arraykey) ...$values): void {}
}

function takes_variadic_arr((function((arraykey)...): void) $f): void {}

function test_mirror((function(...(arraykey)): void) $g): void {
  takes_variadic_arr($g);
}
