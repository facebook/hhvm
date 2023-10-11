<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I { }
class Wrap<T> {
  public function __construct(private T $item) { }
  public function Count():int where T = int { return $this->item; }
}

function AcceptWrappedString(Wrap<string> $x):int {
  $y = 32;
  $z = $y + $x->Count();
  return $z;
}
