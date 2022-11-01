<?hh

function project<Tv, Ts>(
  Traversable<Tv> $traversable,
  (function(Tv): Ts) $projection,
): vec<Tv> {
  foreach ($traversable as $val) {
    $projection($val);
  }
  return vec[];
}

<<__EntryPoint>>
function main(): void {
  project(vec[vec[0]], $x ==> $x['name']);
}
