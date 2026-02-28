<?hh

function identity<T>(T $x): T { return $x; }

function rcvr(
  (function(int): int) $_ ,
  (function(string): string) $_
): void {}

function pass_generic(): void {
  $f = identity<>;
  rcvr($f, $f);
}
