<?hh

function f() {
  $a0 = 1;
  $a = <<__Memoize>> function () use($a0) : int {
    return 1;
  };
}

