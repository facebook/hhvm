<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait TRIMPL {
  private function foo_impl(): bool {
    return false;
  }
}

trait TR {
  use TRIMPL;

  public function foo(): bool {
    return $this->foo_impl();
  }
  public function bar(): bool {
    $this as C;
    return $this->foo_impl();
  }
}

final class C
{
  use TR;
}
