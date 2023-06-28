<?hh

class C {
  public function __construct($x) {
    $this->x = $x;
  }
  public $x;
}

<<__Memoize>> function func() :mixed{

  if (MemoizeRecursion::$counter == 0) return new C(100);
  --MemoizeRecursion::$counter;
  return new C(func()->x + 1);
}

abstract final class MemoizeRecursion {
  public static $counter;
}
<<__EntryPoint>>
function entrypoint_recursion(): void {
  // Copyright 2004-present Facebook. All Rights Reserved.

  MemoizeRecursion::$counter = 10;

  var_dump(func()->x);
  var_dump(func()->x);
}
