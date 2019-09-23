<?hh

function f() {
  $a0 = 1;
  $a = <<__Memoize>> function (): int use($a0) {
    return 1;
  };
}

