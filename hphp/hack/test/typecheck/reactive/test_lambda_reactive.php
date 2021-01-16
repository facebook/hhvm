<?hh // strict
<<__Rx>>
function test(): Rx<(function(): int)> {
  $x = <<__Rx>> function() {
    return 5;
  };
  return $x;
}

// $x is reactive, no error
function test2(): Rx<(function(): int)> {
  $x = <<__Rx>> function() {
    return 5;
  };
  return $x;
}

class GlobalClassName {
  public static int $val = 5;
}

// $x is not reactive, error
function test3(): Rx<(function(): int)> {
  $x = function() {
    return GlobalClassName::$val;
  };
  return $x;
}
