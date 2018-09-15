<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$counter = 10;

class C {
  public function __construct($x) {
    $this->x = $x;
  }
  public $x;
}

<<__Memoize>> function func() {
  global $counter;
  if ($counter == 0) return new C(100);
  --$counter;
  return new C(func()->x + 1);
}

var_dump(func()->x);
var_dump(func()->x);
