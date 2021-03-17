<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

final class TestMutualProtectedTest  {

  public function test(): void {
    $foo = new FooExample();
    $bar = new BarExample();

    $foo->do();
    $bar->do();

    // Leads to a fatal
    $foo->doOn($bar);
  }

}

trait TCommon {
  protected function doCommon(): void {
    return;
  }
}

final class FooExample {
  use TCommon;
  public function do(): void {
      $this->doCommon();
    }

  public function doOn(BarExample $bar): void {
    // Fatal:
    // Call to protected method BarExample::get() from context 'FooExample'
    $bar->doCommon();
  }
}

final class BarExample {
  use TCommon;

  public function do(): void {
      $this->doCommon();
    }
}

<<__EntryPoint>>
function main():void {
  (new TestMutualProtectedTest())->test();
}
