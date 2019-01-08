<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  protected int $batchID;
}

final class B extends A {

  public function __construct(int $batch_id) {
  }

  public function getX(): int {
    return $this->batchID;
  }

}
