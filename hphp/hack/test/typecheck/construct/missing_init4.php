<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  protected int $batchID;
  protected int $A_Test;
}

final class B extends A {
  protected int $batchID = 3;
  protected int $B_test = 2;

  public function __construct(int $batch_id) {
  }

  public function getX(): int {
    return $this->batchID;
  }

}
