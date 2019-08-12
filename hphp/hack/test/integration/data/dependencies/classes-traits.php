<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait ImplementingAbstractBase {
  protected function must_implement(): void {
  }
}

trait T {
  require extends AbstractBase;

  public function routine(): void {
    $this->must_implement();
  }
}

class TraitBase extends AbstractBase {
  use ImplementingAbstractBase;
  use T;
}

function with_traits(TraitBase $arg) : void {
  $arg->routine();
}
