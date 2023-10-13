<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Box<T> {
  public T $value;
  public function __construct(T $v) {
    $this->value = $v;
  }
}

function foo():Box<int> {
  $x = 3;
  $y = new Box($x);
  return $y;
}
