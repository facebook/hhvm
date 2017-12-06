<?hh // strict
<<__Rx>>
function test(): Rx<(function(): int)> {
  $x = function() {
    return 5;
  };
  return $x;
}

// should error
function test2(): Rx<(function(): int)> {
  $x = function() {
    return 5;
  };
  return $x;
}
