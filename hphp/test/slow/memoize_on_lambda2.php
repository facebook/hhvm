<?hh

function f() :mixed{
  $a0 = 1;
  $a = <<__Memoize>> function () use($a0) : int {
    return 1;
  };
}

