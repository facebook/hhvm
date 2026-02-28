<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  protected int $batchID;
}

abstract class B extends A {}

final class C extends B {

  public function __construct(int $batch_id) {
    $this->batchID = $batch_id;
  }

  public function getX(): int {
    return $this->batchID;
  }

}
