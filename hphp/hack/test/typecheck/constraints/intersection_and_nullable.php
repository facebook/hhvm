<?hh // strict

function apply<T>(
  (function(T): T) $func,
  T $x,
): void {
}

function test(): void {
  apply(
    ($x) ==> $x is null ? null : $x + ($x ?? 0.0),
    0.0,
  );
}
