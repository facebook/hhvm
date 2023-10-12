<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait Traitt {
  require extends AbstractClass;
}

abstract class AbstractClass {
  abstract const int THE_CONST;
}

class ActualClass extends AbstractClass {
  const int THE_CONST = 0;
}

final class FinalActualClass extends ActualClass {
  use Traitt;
}
