<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public int $key = 0;
}
function expect<T>(T $x):void { }
function simple_test(Vector<C> $v):void {
    $data = Vector{};
    $data = $data->concat($v);
    $result1 = $data->lastValue();
    $result2 = $data->lastValue();
    if ($result1 !== null) {
      $x1 = $result1->key;
      expect<int>($x1);
    }
    $x2 = $result2?->key;
    expect<?int>($x2);
}
