<?hh

function f(?int $x, Map<string, ?int> $y): void {
  g($x, idx($y, 'foo'));
}

function g<T>(T $_, T $_): void {}
