<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  abstract const int THE_CONST;
}

class ActualClass {
  const int THE_CONST = 0;
}

class FinalActualClass extends ActualClass implements I {
  public function __construct() {}
}
