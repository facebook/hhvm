<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

final class EN {
  use TR;
  final private function asEN(): EN {
    return $this;
  }

  public function doit(): bool { return true; }

}
trait TR {

  final public function foo(): bool {
    if ($this is EN) {
      $note = $this->asEN();
      return $note->doit();
    }
    return false;
  }

  private function asEN(): EN {
    return $this as EN;
  }

}
