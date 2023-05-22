<?hh

function bar(): vec<int> { return vec[0]; }

function foo(): void {
  $id = bar()[0];
  $id is nonnull ? (string) $id : 'null',
}
