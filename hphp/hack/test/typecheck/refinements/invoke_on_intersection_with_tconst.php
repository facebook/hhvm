<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class C {
  const type TID = int;
  final private function __construct(
    private this::TID $id,
  ) {}
  final public function getID(): this::TID {
    return $this->id;
  }
}

function testit<T>(T $x):int {
  if ($x is C) {
    $y = $x->getID();
    return $y;
  }
  return 0;
}
