<?hh

function takes_int(int $_): void {}
function takes_dict(dict<string, mixed> $_): void {}
function takes_tuple((int, dict<string, mixed>) $_): void {}

// Keep the result because only a non-collection flows outside
function dont_invalidate(): void {
  $d = dict['a' => 42];
  takes_int($d['a']);

  $t = tuple(42, dict['a' => 42]);
  takes_int($t[0]);
}

// Invalidate the result because dict flows outside
function invalidate(): void {
  $d = dict['a' => dict[]];
  takes_dict($d['a']);

  $t = tuple(42, dict['a' => 42]);
  takes_dict($t[1]);

  $t = tuple(42, tuple(42, dict['a' => 42]));
  takes_tuple($t[1]);
}
