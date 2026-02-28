<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class B {
  protected int $batchID;

  public function __construct(string $batch_id) {
    switch ($batch_id) {
      case 'test1':
      case 'test2':
        $this->batchID = 1;
        break;
      default:
        $this->batchID  = 3;
    }
  }
}
