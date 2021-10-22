<?hh
<<file:__EnableUnstableFeatures("readonly")>>
function foo(int $x, (function(int): int) $f): int {
  return $f($x);
}

function bar(int $x): void {
  $f = function($x): readonly int {
    return 5;
  };

  foo($x, $f);
  foo($x, function($x): readonly int {
    return $x;
  });
}
