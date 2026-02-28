<?hh

function f() :mixed{
  $a0 = 1;
  $a = <<__Memoize>> function (): int use($a0) {
    return 1;
  };
}

